package com.fletcherw.ledcontroller;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.util.Set;
import java.util.UUID;

public class MainActivity
    extends AppCompatActivity
    implements AdapterView.OnItemSelectedListener, CompoundButton.OnCheckedChangeListener
{
  private BroadcastReceiver btBroadcastReceiver;

  private BluetoothSocket btSocket;
  private OutputStream btOut;

  private boolean btConnected;
  private int selectedPattern;

  private Spinner patternSpinner;
  private Switch powerSwitch;
  private TextView btStatusText;

  public MainActivity() {
    btBroadcastReceiver = null;
    btSocket = null;
    btOut = null;
    btConnected = false;

    selectedPattern = -1;
  }

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

    // Bluetooth
    btStatusText = findViewById(R.id.btStatusText);
    btStatusText.setText(R.string.btDown);

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
  };

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
      parentActivity.btStatusText.setText(R.string.btConnecting);
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
        System.out.println(e.getMessage());
        return null;
      }
    }

    @Override
    protected void onPostExecute(BluetoothSocket btSocket) {
      MainActivity parentActivity = this.parentActivity.get();
      if (parentActivity == null) return;
      if (btSocket == null) {
        parentActivity.btStatusText.setText(R.string.btDown);
      } else {
        parentActivity.btSocket = btSocket;
        try {
          parentActivity.btOut = btSocket.getOutputStream();
        } catch (IOException e) {
          System.out.println(e.getMessage());
        }
        parentActivity.btConnected = true;
        parentActivity.btStatusText.setText(R.string.btUp);
        new StateUpdateTask(btSocket, parentActivity).execute();
      }
    }
  }

  public static class CurrentState {
    public boolean enabled;
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
      try {
        BluetoothSocket btSocket = this.btSocket.get();
        if (btSocket == null) return null;
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
        s.pattern = buffer[1];
        return s;

      } catch (IOException e) {
        e.printStackTrace();
        return null;
      }
    }

    @Override
    protected void onPostExecute(CurrentState s) {
      if (s == null) return;
      MainActivity parentActivity = this.parentActivity.get();
      if (parentActivity == null) return;

      parentActivity.powerSwitch.setChecked(s.enabled);
      parentActivity.patternSpinner.setSelection(s.pattern, true);
      parentActivity.selectedPattern = s.pattern;
    }
  }

  @Override
  public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
    if (!btConnected) return;
    if (pos == selectedPattern) return;
    selectedPattern = pos;
    try {
      btOut.write(0x2);
      btOut.write('S');
      btOut.write(pos);
    } catch (IOException e) {
      Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_LONG).show();
    }
  }

  @Override
  public void onNothingSelected(AdapterView<?> adapterView) {}

  @Override
  public void onCheckedChanged(CompoundButton compoundButton, boolean state) {
    if (!btConnected) return;
    try {
      btOut.write(0x2);
      if (state) {
        btOut.write("P1".getBytes());
      } else {
        btOut.write("P0".getBytes());
      }
    } catch (IOException e) {
      Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_LONG).show();
    }
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    unregisterReceiver(btBroadcastReceiver);
  }
}
