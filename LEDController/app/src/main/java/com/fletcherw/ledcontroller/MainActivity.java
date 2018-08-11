package com.fletcherw.ledcontroller;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;

import java.io.IOException;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.util.Set;
import java.util.UUID;

public class MainActivity
    extends AppCompatActivity
    implements
    AdapterView.OnItemSelectedListener,
    CompoundButton.OnCheckedChangeListener,
    View.OnClickListener,
    SeekBar.OnSeekBarChangeListener
{
  enum BTState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
  }

  private BroadcastReceiver btBroadcastReceiver;

  private BluetoothSocket btSocket;
  private OutputStream btOut;

  private BTState btState;
  private int selectedPattern;
  private boolean power;
  private boolean driverMode;
  private int brightness;
  private int red;
  private int green;
  private int blue;

  private Spinner patternSpinner;
  private Switch powerSwitch;
  private Switch driverModeSwitch;
  private TextView btStatusText;
  private Button btActionButton;

  private SeekBar brightnessBar;
  private TextView brightnessPercentText;

  private SeekBar redBar;
  private SeekBar greenBar;
  private SeekBar blueBar;
  private TextView redLabel;
  private TextView greenLabel;
  private TextView blueLabel;
  private TextView colorBox;

  public MainActivity() {
    btBroadcastReceiver = null;
    btSocket = null;
    btOut = null;
    btState = BTState.DISCONNECTED;

    selectedPattern = -1;
  }

  @SuppressLint("DefaultLocale")
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    // Spinner
    patternSpinner = findViewById(R.id.patternSpinner);
    patternSpinner.setOnItemSelectedListener(this);

    ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
        R.array.patterns_array, android.R.layout.simple_spinner_item);

    // Specify the layout to use when the list of choices appears
    adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

    // Apply the adapter to the spinner
    patternSpinner.setAdapter(adapter);

    // Power Switch
    powerSwitch = findViewById(R.id.powerSwitch);
    powerSwitch.setOnCheckedChangeListener(this);
    powerSwitch.setChecked(false);
    power = false;

    // Mode Switch
    driverModeSwitch = findViewById(R.id.driverModeSwitch);
    driverModeSwitch.setOnCheckedChangeListener(this);
    driverMode = false; // False means Backpack mode
    driverModeSwitch.setChecked(driverMode);

    // Brightness Bar
    brightnessBar = findViewById(R.id.brightnessBar);
    brightnessBar.setMax(255);
    brightnessBar.setOnSeekBarChangeListener(this);
    brightnessPercentText = findViewById(R.id.brightnessPercentText);
    // trigger update of percent text label
    brightness = 255;
    brightnessBar.setProgress(brightness);

    // Color Sliders
    redBar = findViewById(R.id.redSlider);
    greenBar = findViewById(R.id.greenSlider);
    blueBar = findViewById(R.id.blueSlider);
    redBar.setMax(255);
    greenBar.setMax(255);
    blueBar.setMax(255);
    redBar.setOnSeekBarChangeListener(this);
    greenBar.setOnSeekBarChangeListener(this);
    blueBar.setOnSeekBarChangeListener(this);

    redLabel = findViewById(R.id.redValue);
    greenLabel = findViewById(R.id.greenValue);
    blueLabel = findViewById(R.id.blueValue);
    colorBox = findViewById(R.id.colorBox);

    // trigger update of color box and labels
    red = 255;
    green = 255;
    blue = 255;
    redBar.setProgress(red);
    greenBar.setProgress(green);
    blueBar.setProgress(blue);
    updateColorBox();


    // Bluetooth
    btStatusText = findViewById(R.id.btStatusText);
    btStatusText.setOnClickListener(this);
    btActionButton = findViewById(R.id.btReconnectButton);
    btActionButton.setOnClickListener(this);
    setBTState(BTState.DISCONNECTED);

    IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
    btBroadcastReceiver = new BTBroadcastReceiver(this);
    registerReceiver(btBroadcastReceiver, filter);

    BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
    switch (btAdapter.getState()){
      case BluetoothAdapter.STATE_OFF:
      case BluetoothAdapter.STATE_TURNING_OFF:
        Intent enableAdapter = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        startActivityForResult(enableAdapter,1);
        // Broadcast Listener will attempt BT connection when BT is fully on
        break;
      case BluetoothAdapter.STATE_ON:
        new BluetoothConnectTask(this, btAdapter).execute();
        break;
      case BluetoothAdapter.STATE_TURNING_ON:
        break;
    }
  }

  protected void setDriverInputEnabled(boolean enabled) {
    patternSpinner.setEnabled(enabled);
    brightnessBar.setEnabled(enabled);
    redBar.setEnabled(enabled);
    greenBar.setEnabled(enabled);
    blueBar.setEnabled(enabled);
    driverModeSwitch.setEnabled(enabled);
    powerSwitch.setEnabled(enabled);
  }

  protected void setBTState(BTState s) {
    this.btState = s;
    switch (s) {
      case CONNECTED:
        this.btStatusText.setText(R.string.bluetooth_connected);
        this.btStatusText.setTextColor(Color.BLACK);
        this.btActionButton.setText(R.string.bluetooth_update);
        this.btActionButton.setEnabled(true);
        this.setDriverInputEnabled(true);
        break;
      case CONNECTING:
        this.btStatusText.setText(R.string.bluetooth_connecting);
        this.btStatusText.setTextColor(Color.BLACK);
        this.btActionButton.setText(R.string.bluetooth_connect);
        this.btActionButton.setEnabled(false);
        this.setDriverInputEnabled(false);
        break;
      case DISCONNECTED:
        this.btStatusText.setText(R.string.bluetooth_disconnected);
        this.btStatusText.setTextColor(Color.parseColor("#B00020"));
        this.btActionButton.setText(R.string.bluetooth_connect);
        this.btActionButton.setEnabled(true);
        this.setDriverInputEnabled(false);
        break;
    }
  }

  public static class BTBroadcastReceiver extends BroadcastReceiver {
    WeakReference<MainActivity> parentActivity;

    public BTBroadcastReceiver(MainActivity parentActivity) {
      this.parentActivity = new WeakReference<>(parentActivity);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
      final String action = intent.getAction();
      if (action == null) return;

      if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
        final int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
            BluetoothAdapter.ERROR);
        switch (state) {
          case BluetoothAdapter.STATE_ON:
            MainActivity parentActivity = this.parentActivity.get();
            if (parentActivity == null) return;
            new BluetoothConnectTask(parentActivity, BluetoothAdapter.getDefaultAdapter()).execute();
            break;
          default:
            break;
        }
      }
    }
  }

  public static class BluetoothConnectTask extends AsyncTask<Void, Void, BluetoothSocket>
  {
    WeakReference<MainActivity> parentActivity;
    WeakReference<BluetoothAdapter> btAdapter;

    BluetoothConnectTask(MainActivity parentActivity, BluetoothAdapter btAdapter) {
      this.parentActivity = new WeakReference<>(parentActivity);
      this.btAdapter = new WeakReference<>(btAdapter);
    }

    @Override
    protected void onPreExecute() {
      super.onPreExecute();
      MainActivity parentActivity = this.parentActivity.get();
      if (parentActivity == null) return;
      parentActivity.setBTState(BTState.CONNECTING);
    }

    @Override
    protected BluetoothSocket doInBackground(Void... voids) {
      BluetoothAdapter btAdapter = this.btAdapter.get();
      if (btAdapter == null) return null;
      Set<BluetoothDevice> bondedDevices = btAdapter.getBondedDevices();
      final String DEVICE_ADDRESS = "20:17:05:08:53:67";

      BluetoothDevice device = null;
      boolean found = false;

      if (bondedDevices.isEmpty()) {
        System.out.println("No devices found");
        return null;
      } else {
        for (BluetoothDevice iterator : bondedDevices) {
          if (iterator.getAddress().equals(DEVICE_ADDRESS)) {
            device = iterator;
            found = true;
            break;
          }
        }
      }

      if (!found) {
        System.out.println("No matching device found");
        return null;
      }

      UUID port_uuid = new UUID(0x0000110100001000L, 0x800000805F9B34FBL);
      try {
        BluetoothSocket btSocket = device.createRfcommSocketToServiceRecord(port_uuid);
        btSocket.connect();
        return btSocket;
      } catch (IOException e) {
        return null;
      }
    }

    @Override
    protected void onPostExecute(BluetoothSocket btSocket) {
      MainActivity parentActivity = this.parentActivity.get();
      if (parentActivity == null) return;
      if (btSocket == null) {
        parentActivity.setBTState(BTState.DISCONNECTED);
      } else {
        parentActivity.btSocket = btSocket;
        try {
          parentActivity.btOut = btSocket.getOutputStream();
        } catch (IOException e) {
          parentActivity.setBTState(BTState.DISCONNECTED);
        }
        parentActivity.setBTState(BTState.CONNECTED);
        new StateUpdateTask(btSocket, parentActivity).execute();
      }
    }
  }

  public static class CurrentState {
    public boolean enabled;
    public boolean driverMode;
    public int brightness;
    public int red;
    public int green;
    public int blue;
    public int pattern;
  }

  public static class StateUpdateTask extends AsyncTask<Void, Void, CurrentState>
  {
    private WeakReference<BluetoothSocket> btSocket;
    private WeakReference<MainActivity> parentActivity;

    private StateUpdateTask(BluetoothSocket btSocket, MainActivity parentActivity) {
      this.btSocket = new WeakReference<>(btSocket);
      this.parentActivity = new WeakReference<>(parentActivity);
    }

    @Override
    protected CurrentState doInBackground(Void... voids) {
      BluetoothSocket btSocket = this.btSocket.get();
      if (btSocket == null) return null;
      try {
        btSocket.getOutputStream().write(0x1);
        btSocket.getOutputStream().write('G');

        byte[] buffer = new byte[32];
        int length = btSocket.getInputStream().read();
        int read = 0;
        int last_read;
        while (read < length) {
          last_read = btSocket.getInputStream().read(buffer, read, length - read);
          read += last_read;
        }

        for (int i = 0; i < read; i++) {
          System.out.println(((int) buffer[i]) & 0xff);
        }

        CurrentState s = new CurrentState();
        s.enabled = buffer[0] == '1';
        s.driverMode = buffer[1] == 'K';
        s.brightness = buffer[2] & 0xff; // account for signedness
        s.red = buffer[3] & 0xff;
        s.green = buffer[4] & 0xff;
        s.blue = buffer[5] & 0xff;
        s.pattern = buffer[6] & 0xff;

        return s;
      } catch (IOException e) {
        return null;
      }
    }

    @Override
    protected void onPostExecute(CurrentState s) {
      MainActivity parentActivity = this.parentActivity.get();
      if (s == null) {
        if (parentActivity == null) return;
        parentActivity.setBTState(BTState.DISCONNECTED);
        return;
      }

      parentActivity.power = s.enabled;
      parentActivity.driverMode = s.driverMode;
      parentActivity.brightness = s.brightness;
      parentActivity.red = s.red;
      parentActivity.green = s.green;
      parentActivity.blue = s.blue;

      parentActivity.powerSwitch.setChecked(s.enabled);
      parentActivity.brightnessBar.setProgress(s.brightness);
      parentActivity.redBar.setProgress(s.red);
      parentActivity.greenBar.setProgress(s.green);
      parentActivity.blueBar.setProgress(s.blue);
      parentActivity.patternSpinner.setSelection(s.pattern, true);
      parentActivity.selectedPattern = s.pattern;
      parentActivity.syncColorSliderState();
    }
  }

  protected void syncColorSliderState() {
    boolean colorEnabled = selectedPattern == 1 || selectedPattern == 2 ||
                           selectedPattern == 5 || selectedPattern == 6;
    redBar.setEnabled(colorEnabled);
    greenBar.setEnabled(colorEnabled);
    blueBar.setEnabled(colorEnabled);
    if (colorEnabled) updateColorBox();
    else colorBox.setBackgroundColor(Color.LTGRAY);
  }

  @Override
  public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
    if (pos == selectedPattern) return;
    if (btState != BTState.CONNECTED) {
      patternSpinner.setSelection(selectedPattern);
      return;
    }
    try {
      btOut.write(0x2);
      btOut.write('S');
      btOut.write(pos);
      selectedPattern = pos;
      syncColorSliderState();
    } catch (IOException e) {
      patternSpinner.setSelection(selectedPattern);
      setBTState(BTState.DISCONNECTED);
    }
  }

  @Override
  public void onNothingSelected(AdapterView<?> adapterView) {}

  @Override
  public void onCheckedChanged(CompoundButton compoundButton, boolean state) {
    if (compoundButton == powerSwitch) {
      if (btState != BTState.CONNECTED) {
        powerSwitch.setChecked(power);
        return;
      }
      try {
        btOut.write('\2');
        btOut.write('P');
        btOut.write(state ? '1' : '0');
      } catch (IOException e) {
        powerSwitch.setChecked(power);
        setBTState(BTState.DISCONNECTED);
      }
      power = state;
    } else if (compoundButton == driverModeSwitch) {
      if (btState != BTState.CONNECTED) {
        driverModeSwitch.setChecked(driverMode);
        return;
      }
      try {
        btOut.write('\2');
        btOut.write('M');
        btOut.write(state ? 'K' : 'G');
      } catch (IOException e) {
        driverModeSwitch.setChecked(driverMode);
        setBTState(BTState.DISCONNECTED);
      }
      driverMode = state;
    }
  }

  @Override
  public void onClick(View view) {
    if (view == btActionButton) {
      if (btState == BTState.DISCONNECTED) {
        new BluetoothConnectTask(this, BluetoothAdapter.getDefaultAdapter()).execute();
        btActionButton.setActivated(false);
      } else if (btState == BTState.CONNECTED) {
        new StateUpdateTask(this.btSocket, this).execute();
      }
    }
  }

  private void updateColorBox() {
    int red = redBar.getProgress();
    int green = greenBar.getProgress();
    int blue = blueBar.getProgress();
    colorBox.setBackgroundColor(Color.rgb(red, green, blue));
  }

  @SuppressLint("DefaultLocale")
  @Override
  public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
    if (seekBar == brightnessBar) {
      int percentBrightness = progress * 100 / 255;
      brightnessPercentText.setText(String.format("%d%%", percentBrightness));
    } else if (seekBar == redBar) {
      redLabel.setText(String.format("%d", progress));
      updateColorBox();
    } else if (seekBar == greenBar) {
      greenLabel.setText(String.format("%d", progress));
      updateColorBox();
    } else if (seekBar == blueBar) {
      blueLabel.setText(String.format("%d", progress));
      updateColorBox();
    }
  }

  @Override
  public void onStartTrackingTouch(SeekBar seekBar) {}

  @Override
  public void onStopTrackingTouch(SeekBar seekBar) {
    if (seekBar == brightnessBar) {
      if (btState != BTState.CONNECTED) {
        brightnessBar.setProgress(brightness);
        return;
      }
      try {
        btOut.write(0x2);
        btOut.write('B');
        btOut.write(brightnessBar.getProgress());
        brightness = brightnessBar.getProgress();
      } catch (IOException e) {
        brightnessBar.setProgress(brightness);
        setBTState(BTState.DISCONNECTED);
      }
    } else if (seekBar == redBar || seekBar == greenBar || seekBar == blueBar) {
      if (btState != BTState.CONNECTED) {
        redBar.setProgress(red);
        greenBar.setProgress(green);
        blueBar.setProgress(blue);
      }
      try {
        btOut.write(0x4);
        btOut.write('C');
        btOut.write(redBar.getProgress());
        btOut.write(greenBar.getProgress());
        btOut.write(blueBar.getProgress());
        red = redBar.getProgress();
        green = greenBar.getProgress();
        blue = blueBar.getProgress();
      } catch (IOException e) {
        redBar.setProgress(red);
        greenBar.setProgress(green);
        blueBar.setProgress(blue);
        setBTState(BTState.DISCONNECTED);
      }
    }
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    unregisterReceiver(btBroadcastReceiver);
    try {
      if (btSocket != null) btSocket.close();
    } catch (IOException e) {
      System.out.println(e.getMessage());
    }
  }
}
