package com.fletcherw.ledcontroller;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
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
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.util.Set;
import java.util.UUID;

public class MainActivity
    extends AppCompatActivity
    implements AdapterView.OnItemSelectedListener, CompoundButton.OnCheckedChangeListener
{
  private BluetoothSocket btSocket;
  private InputStream btInput;
  private OutputStream btOutput;
  private boolean btConnected;

  private Spinner patternSpinner;
  private Switch powerSwitch;
  private TextView btStatusText;

  public MainActivity() {

    btSocket = null;
    btInput = null;
    btOutput = null;
    btConnected = false;
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
    connectToBluetooth();
  }

  public static class StateUpdateTask extends AsyncTask<Void, Void, Boolean> {

    private WeakReference<OutputStream> os;
    private WeakReference<InputStream> is;
    private WeakReference<Switch> powerSwitch;

    private StateUpdateTask(Switch powerSwitch, OutputStream os, InputStream is) {
      this.os = new WeakReference<OutputStream>(os);
      this.is = new WeakReference<InputStream>(is);
      this.powerSwitch = new WeakReference<Switch>(powerSwitch);
    }

    @Override
    protected Boolean doInBackground(Void... voids) {
      try {
        os.get().write(0x2);
        os.get().write("GP".getBytes());

        byte[] buffer = new byte[32];
        int length = is.get().read();
        int read = 0;
        int last_read = 0;
        while (read < length) {
          last_read = is.get().read(buffer, read, length - read);
          read += last_read;
        }

        return buffer[0] == '1';

      } catch (Exception e) {
        e.printStackTrace();
      }

      return false;
    }

    @Override
    protected void onPostExecute(Boolean b) {
      this.powerSwitch.get().setChecked(b);
    }
  }

  void connectToBluetooth() {
    BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    if (!bluetoothAdapter.isEnabled()) {
      Intent enableAdapter = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
      startActivityForResult(enableAdapter,1);
    }

    Set<BluetoothDevice> bondedDevices = bluetoothAdapter.getBondedDevices();
    final String DEVICE_ADDRESS = "20:17:05:08:53:67";

    BluetoothDevice device = null;
    boolean found = false;

    if (bondedDevices.isEmpty()) {
      Toast.makeText(getApplicationContext(),"No devices found", Toast.LENGTH_SHORT).show();
      return;
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
      Toast.makeText(getApplicationContext(),"No matching device found", Toast.LENGTH_SHORT).show();
      return;
    }

    UUID port_uuid = new UUID(0x0000110100001000L, 0x800000805F9B34FBL);
    try {
      btSocket = device.createRfcommSocketToServiceRecord(port_uuid);
      btSocket.connect();
      btInput = btSocket.getInputStream();
      btOutput = btSocket.getOutputStream();
    } catch (IOException e) {
      Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_LONG).show();
      return;
    }
    btConnected = true;
    Toast.makeText(getApplicationContext(), "Connected!", Toast.LENGTH_LONG).show();
    new StateUpdateTask(powerSwitch, btOutput, btInput).execute();
  }


  @Override
  public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
    try {
      btOutput.write(0x2);
      btOutput.write('S');
      btOutput.write(pos);
    } catch (IOException e) {
      Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_LONG).show();
    }
  }

  @Override
  public void onNothingSelected(AdapterView<?> adapterView) {

  }

  @Override
  public void onCheckedChanged(CompoundButton compoundButton, boolean state) {
    if (!btConnected) return;
    try {
      if (state) {
        btOutput.write("\2P1".getBytes());
        btOutput.flush();
      } else {
        btOutput.write("\2P0".getBytes());
        btOutput.flush();
      }
    } catch (IOException e) {
      Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_LONG).show();
    }
  }
}
