package com.fletcherw.ledcontroller;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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

  private Spinner patternSpinner;
  private Switch powerSwitch;
  private TextView btStatusText;
  private Button btReconnectButton;

  private SeekBar dimmerBar;
  private TextView dimmerPercentText;

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

    // Dimmer
    dimmerBar = findViewById(R.id.dimmerBar);
    dimmerBar.setMax(255);
    dimmerBar.setOnSeekBarChangeListener(this);
    dimmerPercentText = findViewById(R.id.dimmerPercentText);
    // trigger update of percent text label
    dimmerBar.setProgress(255);

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
    redBar.setProgress(255);
    greenBar.setProgress(255);
    blueBar.setProgress(255);
    updateColorBox();


    // Bluetooth
    btStatusText = findViewById(R.id.btStatusText);
    btStatusText.setOnClickListener(this);
    btReconnectButton = findViewById(R.id.btReconnectButton);
    btReconnectButton.setOnClickListener(this);
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

  protected void setBTState(BTState s) {
    this.btState = s;
    switch (s) {
      case CONNECTED:
        this.btStatusText.setText(R.string.bluetooth_connected);
        this.btStatusText.setTextColor(Color.BLACK);
        this.btReconnectButton.setEnabled(false);
        break;
      case CONNECTING:
        this.btStatusText.setText(R.string.bluetooth_connecting);
        this.btStatusText.setTextColor(Color.BLACK);
        this.btReconnectButton.setEnabled(false);
        break;
      case DISCONNECTED:
        this.btStatusText.setText(R.string.bluetooth_disconnected);
        this.btStatusText.setTextColor(Color.parseColor("#B00020"));
        this.btReconnectButton.setEnabled(true);
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
        btSocket.getOutputStream().write(0x2);
        btSocket.getOutputStream().write("GP".getBytes());

        byte[] buffer = new byte[32];
        int length = btSocket.getInputStream().read();
        int read = 0;
        int last_read;
        while (read < length) {
          last_read = btSocket.getInputStream().read(buffer, read, length - read);
          read += last_read;
        }

        CurrentState s = new CurrentState();
        s.enabled = buffer[0] == '1';
        s.brightness = buffer[1] & 0xff; // account for signedness
        s.red = buffer[2] & 0xff;
        s.green = buffer[3] & 0xff;
        s.blue = buffer[4] & 0xff;
        s.pattern = buffer[5] & 0xff;

        return s;
      } catch (IOException e) {
        MainActivity parentActivity = this.parentActivity.get();
        if (parentActivity != null) parentActivity.setBTState(BTState.DISCONNECTED);
        return null;
      }
    }

    @Override
    protected void onPostExecute(CurrentState s) {
      if (s == null) return;
      MainActivity parentActivity = this.parentActivity.get();
      if (parentActivity == null) return;

      parentActivity.powerSwitch.setChecked(s.enabled);
      parentActivity.dimmerBar.setProgress(s.brightness);
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
    selectedPattern = pos;
    syncColorSliderState();

    if (btState != BTState.CONNECTED) return;
    try {
      btOut.write(0x2);
      btOut.write('S');
      btOut.write(pos);
    } catch (IOException e) {
      setBTState(BTState.DISCONNECTED);
    }
  }

  @Override
  public void onNothingSelected(AdapterView<?> adapterView) {}

  @Override
  public void onCheckedChanged(CompoundButton compoundButton, boolean state) {
    if (compoundButton != powerSwitch) return;
    if (btState != BTState.CONNECTED) return;
    try {
      if (state) {
        btOut.write("\2P1".getBytes());
      } else {
        btOut.write("\2P0".getBytes());
      }
    } catch (IOException e) {
      setBTState(BTState.DISCONNECTED);
    }
  }

  @Override
  public void onClick(View view) {
    if (view == btReconnectButton) {
      if (btState == BTState.DISCONNECTED) {
        new BluetoothConnectTask(this, BluetoothAdapter.getDefaultAdapter()).execute();
        btReconnectButton.setActivated(false);
      }
    } else if (view == btStatusText) {
      if (btState == BTState.CONNECTED) {
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
    if (seekBar == dimmerBar) {
      int percentBrightness = progress * 100 / 255;
      dimmerPercentText.setText(String.format("%d%%", percentBrightness));
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
    if (btState != BTState.CONNECTED) return;
    if (seekBar == dimmerBar) {
      try {
        btOut.write(0x2);
        btOut.write('B');
        btOut.write(dimmerBar.getProgress());
      } catch (IOException e) {
        setBTState(BTState.DISCONNECTED);
      }
    } else if (seekBar == redBar || seekBar == greenBar || seekBar == blueBar) {
      try {
        btOut.write(0x4);
        btOut.write('C');
        btOut.write(redBar.getProgress());
        btOut.write(greenBar.getProgress());
        btOut.write(blueBar.getProgress());
      } catch (IOException e) {
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
