package com.fletcherw.ledcontroller;

import android.Manifest;
import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Build;
import android.util.Log;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import java.io.IOException;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
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
  private static final String TAG = "MainActivity";

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
  private boolean randomMode;
  private int brightness;
  private int red;
  private int green;
  private int blue;

  private TextView btStatusText;
  private Button btActionButton;

  private Spinner patternSpinner;
  private ArrayAdapter<PatternInfo> patternAdapter;
  private SwitchCompat powerSwitch;

  private SwitchCompat randomSwitch;

  private SeekBar brightnessBar;
  private TextView brightnessPercentText;

  private SeekBar redBar;
  private SeekBar greenBar;
  private SeekBar blueBar;
  private TextView redLabel;
  private TextView greenLabel;
  private TextView blueLabel;
  private TextView colorBox;

  private TextView errorText;

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

    // Pattern names are fetched from the driver on connect; start empty.
    patternAdapter =
        new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, new ArrayList<>());
    patternAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
    patternSpinner.setAdapter(patternAdapter);

    // Power Switch
    powerSwitch = findViewById(R.id.powerSwitch);
    powerSwitch.setOnCheckedChangeListener(this);
    powerSwitch.setChecked(false);
    power = false;

    // Random Switch
    randomSwitch = findViewById(R.id.randomSwitch);
    randomSwitch.setOnCheckedChangeListener(this);
    randomMode = false;
    randomSwitch.setChecked(randomMode);

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

    // Error label
    errorText = findViewById(R.id.errorText);

    // Bluetooth
    btStatusText = findViewById(R.id.btStatusText);
    btStatusText.setOnClickListener(this);
    btActionButton = findViewById(R.id.btReconnectButton);
    btActionButton.setOnClickListener(this);
    setBTState(BTState.DISCONNECTED);

    IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
    btBroadcastReceiver = new BTBroadcastReceiver(this);
    registerReceiver(btBroadcastReceiver, filter);

    requestBluetoothPermissionThenCheckState();
  }

  private static final int REQUEST_BLUETOOTH_CONNECT = 100;

  private void requestBluetoothPermissionThenCheckState() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S
        && ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT)
            != PackageManager.PERMISSION_GRANTED) {
      ActivityCompat.requestPermissions(
          this, new String[] {Manifest.permission.BLUETOOTH_CONNECT}, REQUEST_BLUETOOTH_CONNECT);
      return;
    }
    checkBTState();
  }

  @Override
  public void onRequestPermissionsResult(
      int requestCode, String[] permissions, int[] grantResults) {
    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    if (requestCode == REQUEST_BLUETOOTH_CONNECT
        && grantResults.length > 0
        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
      checkBTState();
    } else {
      errorText.setText(R.string.bluetooth_permission_denied);
    }
  }

  @Override
  protected void onResume() {
    super.onResume();
    if (btState == BTState.CONNECTED) new StateUpdateTask(btSocket, this).execute();
  }

  protected void setDriverInputEnabled(boolean enabled) {
    patternSpinner.setEnabled(enabled);
    brightnessBar.setEnabled(enabled);
    redBar.setEnabled(enabled);
    greenBar.setEnabled(enabled);
    blueBar.setEnabled(enabled);
    powerSwitch.setEnabled(enabled);
    randomSwitch.setEnabled(enabled);
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

  private void checkBTState() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S
        && ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT)
            != PackageManager.PERMISSION_GRANTED) {
      errorText.setText(R.string.bluetooth_permission_denied);
      return;
    }
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
          case BluetoothAdapter.STATE_ON: {
            MainActivity parentActivity = this.parentActivity.get();
            if (parentActivity == null) return;
            new BluetoothConnectTask(parentActivity, BluetoothAdapter.getDefaultAdapter()).execute();
            break;
          }
          case BluetoothAdapter.STATE_DISCONNECTED:
          case BluetoothAdapter.STATE_DISCONNECTING:
          case BluetoothAdapter.STATE_OFF:
          case BluetoothAdapter.STATE_TURNING_OFF: {
            MainActivity parentActivity = this.parentActivity.get();
            if (parentActivity == null) return;
            parentActivity.setBTState(BTState.DISCONNECTED);
          }
          default:
            break;
        }
      }
    }
  }

  private static class Errorable<T> {
    public T result;
    public String error;

    public Errorable() {
      this.error = null;
      this.result = null;
    }
  }

  public static class BluetoothConnectTask extends AsyncTask<Void, Void, Errorable<BluetoothSocket>>
  {
    WeakReference<MainActivity> parentActivity;
    WeakReference<BluetoothAdapter> btAdapter;

    BluetoothConnectTask(MainActivity parentActivity, BluetoothAdapter btAdapter) {
      this.parentActivity = new WeakReference<>(parentActivity);
      this.btAdapter = new WeakReference<>(btAdapter);
      parentActivity.errorText.setText(R.string.error_default);
    }

    @Override
    protected void onPreExecute() {
      super.onPreExecute();
      MainActivity parentActivity = this.parentActivity.get();
      if (parentActivity == null) return;
      parentActivity.setBTState(BTState.CONNECTING);
    }

    @Override
    protected Errorable<BluetoothSocket> doInBackground(Void... voids) {
      Errorable<BluetoothSocket> result = new Errorable<>();
      BluetoothAdapter btAdapter = this.btAdapter.get();
      MainActivity parentActivity = this.parentActivity.get();
      if (btAdapter == null || parentActivity == null) {
        result.error = "No Bluetooth Adapter Found";
        return result;
      }
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S
          && ContextCompat.checkSelfPermission(
                  parentActivity, Manifest.permission.BLUETOOTH_CONNECT)
              != PackageManager.PERMISSION_GRANTED) {
        result.error = "Bluetooth permission denied";
        return result;
      }
      Set<BluetoothDevice> bondedDevices = btAdapter.getBondedDevices();
      final String DEVICE_ADDRESS = "20:17:05:08:53:67";

      BluetoothDevice device = null;
      boolean found = false;

      if (bondedDevices.isEmpty()) {
        result.error = "No devices found";
        return result;
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
        result.error = "No matching device found";
        return result;
      }

      UUID port_uuid = new UUID(0x0000110100001000L, 0x800000805F9B34FBL);
      try {
        BluetoothSocket btSocket = device.createRfcommSocketToServiceRecord(port_uuid);
        btSocket.connect();
        result.result = btSocket;
      } catch (IOException e) {
        result.error = e.getMessage();
      }
      return result;
    }

    @Override
    protected void onPostExecute(Errorable<BluetoothSocket> result) {
      MainActivity parentActivity = this.parentActivity.get();
      if (parentActivity == null) return;
      if (result == null) {
        parentActivity.setBTState(BTState.DISCONNECTED);
      } else if (result.error != null) {
        parentActivity.setBTState(BTState.DISCONNECTED);
        parentActivity.errorText.setText(result.error);
      } else if (result.result != null) {
        BluetoothSocket btSocket = result.result;
        parentActivity.btSocket = btSocket;
        try {
          parentActivity.btOut = btSocket.getOutputStream();
          // Always run in bike mode; there is no user-facing driver mode toggle.
          parentActivity.btOut.write('\2');
          parentActivity.btOut.write('M');
          parentActivity.btOut.write('K');
        } catch (IOException e) {
          parentActivity.errorText.setText(e.getMessage());
          parentActivity.setBTState(BTState.DISCONNECTED);
        }
        parentActivity.setBTState(BTState.CONNECTED);
        new PatternListTask(btSocket, parentActivity).execute();
      }
    }
  }

  public static class PatternInfo {
    public final String name;
    public final boolean canSetColor;

    public PatternInfo(String name, boolean canSetColor) {
      this.name = name;
      this.canSetColor = canSetColor;
    }

    @Override
    public String toString() {
      return name;
    }
  }

  public static class PatternListTask extends AsyncTask<Void, Void, Errorable<List<PatternInfo>>>
  {
    private WeakReference<BluetoothSocket> btSocket;
    private WeakReference<MainActivity> parentActivity;

    private PatternListTask(BluetoothSocket btSocket, MainActivity parentActivity) {
      this.btSocket = new WeakReference<>(btSocket);
      this.parentActivity = new WeakReference<>(parentActivity);
      parentActivity.errorText.setText(R.string.error_default);
    }

    @Override
    protected Errorable<List<PatternInfo>> doInBackground(Void... voids) {
      BluetoothSocket btSocket = this.btSocket.get();
      Errorable<List<PatternInfo>> result = new Errorable<>();
      if (btSocket == null) {
        result.error = "BluetoothSocket is null";
        return result;
      }
      try {
        btSocket.getOutputStream().write(0x1);
        btSocket.getOutputStream().write('L');

        byte[] buffer = new byte[255];
        int length = btSocket.getInputStream().read();
        int read = 0;
        int last_read;
        while (read < length) {
          last_read = btSocket.getInputStream().read(buffer, read, length - read);
          read += last_read;
        }

        int patternCount = buffer[1] & 0xff;
        List<PatternInfo> patterns = new ArrayList<>(patternCount);
        int cursor = 2;
        for (int i = 0; i < patternCount; i++) {
          int nameLength = buffer[cursor] & 0xff;
          cursor++;
          String name = new String(buffer, cursor, nameLength, "US-ASCII");
          cursor += nameLength;
          boolean canSetColor = buffer[cursor] == '1';
          cursor++;
          patterns.add(new PatternInfo(name, canSetColor));
        }

        result.result = patterns;
        return result;
      } catch (IOException e) {
        result.error = e.getMessage();
        return result;
      }
    }

    @Override
    protected void onPostExecute(Errorable<List<PatternInfo>> result) {
      MainActivity parentActivity = this.parentActivity.get();
      BluetoothSocket btSocket = this.btSocket.get();
      if (parentActivity == null) return;
      if (result == null) {
        parentActivity.setBTState(BTState.DISCONNECTED);
      } else if (result.error != null) {
        parentActivity.setBTState(BTState.DISCONNECTED);
        parentActivity.errorText.setText(result.error);
      } else if (result.result != null) {
        parentActivity.patternAdapter.clear();
        parentActivity.patternAdapter.addAll(result.result);
        if (btSocket != null) new StateUpdateTask(btSocket, parentActivity).execute();
      }
    }
  }

  public static class CurrentState {
    public boolean enabled;
    public boolean randomMode;
    public int brightness;
    public int red;
    public int green;
    public int blue;
    public int pattern;
  }

  public static class StateUpdateTask extends AsyncTask<Void, Void, Errorable<CurrentState>>
  {
    private WeakReference<BluetoothSocket> btSocket;
    private WeakReference<MainActivity> parentActivity;

    private StateUpdateTask(BluetoothSocket btSocket, MainActivity parentActivity) {
      this.btSocket = new WeakReference<>(btSocket);
      this.parentActivity = new WeakReference<>(parentActivity);
      parentActivity.errorText.setText(R.string.error_default);
    }

    @Override
    protected Errorable<CurrentState> doInBackground(Void... voids) {
      BluetoothSocket btSocket = this.btSocket.get();
      Errorable<CurrentState> result = new Errorable<>();
      if (btSocket == null) {
        result.error = "BluetoothSocket is null";
        return result;
      }
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

        CurrentState s = new CurrentState();
        s.enabled = buffer[1] == '1';
        s.randomMode = buffer[3] == '1';
        s.brightness = buffer[4] & 0xff; // account for signedness
        s.red = buffer[5] & 0xff;
        s.green = buffer[6] & 0xff;
        s.blue = buffer[7] & 0xff;
        s.pattern = buffer[8] & 0xff;

        result.result = s;
        return result;
      } catch (IOException e) {
        result.error = e.getMessage();
        return result;
      }
    }

    @Override
    protected void onPostExecute(Errorable<CurrentState> result) {
      MainActivity parentActivity = this.parentActivity.get();
      if (parentActivity == null) return;
      if (result == null) {
        parentActivity.setBTState(BTState.DISCONNECTED);
      } else if (result.error != null) {
        parentActivity.setBTState(BTState.DISCONNECTED);
        parentActivity.errorText.setText(result.error);
      } else if (result.result != null) {
        CurrentState s = result.result;
        parentActivity.power = s.enabled;
        parentActivity.randomMode = s.randomMode;
        parentActivity.brightness = s.brightness;
        parentActivity.red = s.red;
        parentActivity.green = s.green;
        parentActivity.blue = s.blue;

        parentActivity.powerSwitch.setChecked(s.enabled);
        parentActivity.randomSwitch.setChecked(s.randomMode);
        parentActivity.brightnessBar.setProgress(s.brightness);
        parentActivity.redBar.setProgress(s.red);
        parentActivity.greenBar.setProgress(s.green);
        parentActivity.blueBar.setProgress(s.blue);
        if (s.pattern >= 0 && s.pattern < parentActivity.patternAdapter.getCount()) {
          parentActivity.patternSpinner.setSelection(s.pattern, true);
          parentActivity.selectedPattern = s.pattern;
        } else {
          String message = "Received out-of-range pattern index from device: " + s.pattern;
          Log.w(TAG, message);
          parentActivity.errorText.setText(message);
        }
        parentActivity.syncColorSliderState();
      }
    }
  }

  protected void syncColorSliderState() {
    boolean colorEnabled = selectedPattern >= 0 && selectedPattern < patternAdapter.getCount()
        && patternAdapter.getItem(selectedPattern).canSetColor;
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
      errorText.setText(e.getMessage());
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
        errorText.setText(e.getMessage());
        powerSwitch.setChecked(power);
        setBTState(BTState.DISCONNECTED);
      }
      power = state;
    } else if (compoundButton == randomSwitch) {
      if (btState != BTState.CONNECTED) {
        randomSwitch.setChecked(randomMode);
        return;
      }
      try {
        btOut.write('\2');
        btOut.write('R');
        btOut.write(state ? '1' : '0');
      } catch (IOException e) {
        errorText.setText(e.getMessage());
        randomSwitch.setChecked(randomMode);
        setBTState(BTState.DISCONNECTED);
      }
      randomMode = state;
    }
  }

  @Override
  public void onClick(View view) {
    if (view == btActionButton) {
      if (btState == BTState.DISCONNECTED) {
        checkBTState();
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
        errorText.setText(e.getMessage());
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
        errorText.setText(e.getMessage());
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
