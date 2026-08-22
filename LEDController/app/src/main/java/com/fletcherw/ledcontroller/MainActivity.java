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
import android.os.SystemClock;
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
import java.io.InputStream;
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
        InputStream in = btSocket.getInputStream();
        int length = readLengthPrefix(in);
        if (length < 2) {
          throw new IOException("Malformed pattern list of length " + length);
        }
        readFully(in, buffer, length);

        int patternCount = buffer[1] & 0xff;
        List<PatternInfo> patterns = new ArrayList<>(patternCount);
        int cursor = 2;
        for (int i = 0; i < patternCount; i++) {
          // Every entry is a name-length byte, the name, and a color-support
          // byte; bail out instead of running off the end of a corrupt frame.
          if (cursor >= length) throw new IOException("Truncated pattern list");
          int nameLength = buffer[cursor] & 0xff;
          cursor++;
          if (cursor + nameLength >= length) throw new IOException("Truncated pattern list");
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

  // How long to wait for a response before giving up and dropping the connection.
  // RFCOMM streams ignore setSoTimeout, so reads are bounded by polling
  // available() against a deadline instead of blocking in read().
  private static final int READ_TIMEOUT_MS = 2000;

  /**
   * Fills buffer[0..len) from the stream, failing rather than blocking forever if the driver
   * stops responding mid-response or closes the connection.
   */
  private static void readFully(InputStream in, byte[] buffer, int len) throws IOException {
    long deadline = SystemClock.elapsedRealtime() + READ_TIMEOUT_MS;
    int read = 0;
    while (read < len) {
      if (in.available() <= 0) {
        if (SystemClock.elapsedRealtime() >= deadline) {
          throw new IOException("Timed out waiting for a response from the driver");
        }
        try {
          Thread.sleep(10);
        } catch (InterruptedException e) {
          Thread.currentThread().interrupt();
          throw new IOException("Interrupted while reading a response");
        }
        continue;
      }
      int lastRead = in.read(buffer, read, len - read);
      // read() returns -1 at EOF; without this the loop below would spin
      // backwards forever and stale buffer contents would parse as real state.
      if (lastRead < 0) throw new IOException("Connection closed by the driver");
      read += lastRead;
    }
  }

  /** Reads the single-byte length prefix that starts every response. */
  private static int readLengthPrefix(InputStream in) throws IOException {
    byte[] prefix = new byte[1];
    readFully(in, prefix, 1);
    return prefix[0] & 0xff;
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

  // Shared by any command whose response has the same shape as 'G' (currently 'G' and 'S').
  private static Errorable<CurrentState> readCurrentState(BluetoothSocket btSocket) {
    Errorable<CurrentState> result = new Errorable<>();
    if (btSocket == null) {
      result.error = "BluetoothSocket is null";
      return result;
    }
    try {
      byte[] buffer = new byte[32];
      InputStream in = btSocket.getInputStream();
      int length = readLengthPrefix(in);
      if (length < 8 || length > buffer.length) {
        throw new IOException("Malformed state response of length " + length);
      }
      readFully(in, buffer, length);

      CurrentState s = new CurrentState();
      s.enabled = buffer[1] == '1';
      s.randomMode = buffer[2] == '1';
      s.brightness = buffer[3] & 0xff; // account for signedness
      s.red = buffer[4] & 0xff;
      s.green = buffer[5] & 0xff;
      s.blue = buffer[6] & 0xff;
      s.pattern = buffer[7] & 0xff;

      result.result = s;
      return result;
    } catch (IOException e) {
      result.error = e.getMessage();
      return result;
    }
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
      if (btSocket == null) {
        Errorable<CurrentState> result = new Errorable<>();
        result.error = "BluetoothSocket is null";
        return result;
      }
      try {
        btSocket.getOutputStream().write(0x1);
        btSocket.getOutputStream().write('G');
      } catch (IOException e) {
        Errorable<CurrentState> result = new Errorable<>();
        result.error = e.getMessage();
        return result;
      }
      return readCurrentState(btSocket);
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
        parentActivity.applyCurrentState(result.result);
      }
    }
  }

  public static class SetPatternTask extends AsyncTask<Void, Void, Errorable<CurrentState>>
  {
    private WeakReference<BluetoothSocket> btSocket;
    private WeakReference<MainActivity> parentActivity;
    private int pos;
    private int previousPattern;

    private SetPatternTask(
        BluetoothSocket btSocket, MainActivity parentActivity, int pos, int previousPattern) {
      this.btSocket = new WeakReference<>(btSocket);
      this.parentActivity = new WeakReference<>(parentActivity);
      this.pos = pos;
      this.previousPattern = previousPattern;
      parentActivity.errorText.setText(R.string.error_default);
    }

    @Override
    protected Errorable<CurrentState> doInBackground(Void... voids) {
      BluetoothSocket btSocket = this.btSocket.get();
      if (btSocket == null) {
        Errorable<CurrentState> result = new Errorable<>();
        result.error = "BluetoothSocket is null";
        return result;
      }
      try {
        btSocket.getOutputStream().write(0x2);
        btSocket.getOutputStream().write('S');
        btSocket.getOutputStream().write(pos);
      } catch (IOException e) {
        Errorable<CurrentState> result = new Errorable<>();
        result.error = e.getMessage();
        return result;
      }
      return readCurrentState(btSocket);
    }

    @Override
    protected void onPostExecute(Errorable<CurrentState> result) {
      MainActivity parentActivity = this.parentActivity.get();
      if (parentActivity == null) return;
      if (result == null) {
        parentActivity.setBTState(BTState.DISCONNECTED);
        parentActivity.patternSpinner.setSelection(previousPattern);
      } else if (result.error != null) {
        parentActivity.setBTState(BTState.DISCONNECTED);
        parentActivity.errorText.setText(result.error);
        parentActivity.patternSpinner.setSelection(previousPattern);
      } else if (result.result != null) {
        parentActivity.applyCurrentState(result.result);
      }
    }
  }

  protected void applyCurrentState(CurrentState s) {
    power = s.enabled;
    randomMode = s.randomMode;
    brightness = s.brightness;
    red = s.red;
    green = s.green;
    blue = s.blue;

    powerSwitch.setChecked(s.enabled);
    randomSwitch.setChecked(s.randomMode);
    brightnessBar.setProgress(s.brightness);
    redBar.setProgress(s.red);
    greenBar.setProgress(s.green);
    blueBar.setProgress(s.blue);
    if (s.pattern >= 0 && s.pattern < patternAdapter.getCount()) {
      patternSpinner.setSelection(s.pattern, true);
      selectedPattern = s.pattern;
    } else {
      String message = "Received out-of-range pattern index from device: " + s.pattern;
      Log.w(TAG, message);
      errorText.setText(message);
    }
    syncColorSliderState();
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
    new SetPatternTask(btSocket, this, pos, selectedPattern).execute();
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
        new PatternListTask(this.btSocket, this).execute();
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
