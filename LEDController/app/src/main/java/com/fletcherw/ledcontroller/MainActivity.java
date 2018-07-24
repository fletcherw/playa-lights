package com.fletcherw.ledcontroller;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Adapter;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.Spinner;
import android.widget.Toast;

import static android.app.PendingIntent.getActivity;

public class MainActivity
    extends AppCompatActivity
    implements AdapterView.OnItemSelectedListener, CompoundButton.OnCheckedChangeListener
{
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
  }

  @Override
  public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
    String selected = (String) parent.getItemAtPosition(pos);
    Toast.makeText(this, selected, Toast.LENGTH_SHORT).show();
  }

  @Override
  public void onNothingSelected(AdapterView<?> adapterView) {

  }

  @Override
  public void onCheckedChanged(CompoundButton compoundButton, boolean state) {
    
  }
}
