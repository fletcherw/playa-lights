package com.fletcherw.ledcontroller;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
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

    Spinner spinner = findViewById(R.id.spinner);
    spinner.setOnItemSelectedListener(this);

    ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
        R.array.planets_array, android.R.layout.simple_spinner_item);

    // Specify the layout to use when the list of choices appears
    adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

    // Apply the adapter to the spinner
    spinner.setAdapter(adapter);


    Switch toggle = findViewById(R.id.switch1);
    toggle.setOnCheckedChangeListener(this);

    // Setup Bluetooth
    connectToBluetooth();
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

    UUID PORT_UUID = new UUID(0x0000110100001000L, 0x800000805F9B34FBL);
    try {
      btSocket = device.createRfcommSocketToServiceRecord(PORT_UUID);
      btSocket.connect();
      btInput = btSocket.getInputStream();
      btOutput = btSocket.getOutputStream();
    } catch (IOException e) {
      Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_LONG).show();
      return;
    }
    btConnected = true;
    Toast.makeText(getApplicationContext(), "Connected!", Toast.LENGTH_LONG).show();
  }


  @Override
  public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
    //String selected = (String) parent.getItemAtPosition(pos);
    //Toast.makeText(this, selected, Toast.LENGTH_SHORT).show();
  }

  @Override
  public void onNothingSelected(AdapterView<?> adapterView) {

  }

  @Override
  public void onCheckedChanged(CompoundButton compoundButton, boolean state) {
    if (!btConnected) return;
    try {
      if (state) {
        btOutput.write("P1".getBytes());
        btOutput.flush();
      } else {
        btOutput.write("P0".getBytes());
        btOutput.flush();
      }
    } catch (IOException e) {
      Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_LONG).show();
    }
  }
}
