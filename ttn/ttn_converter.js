function Converter(decoded, port) {
  // Merge, split or otherwise
  // mutate decoded fields.
  var converted = {};

  //Sensor IDs
// Measurement status
converted["6061262677a88b001b990ddd"] = decoded.sessionStatus;
// Active probes
converted["6061262677a88b001b990ddc"] = decoded.probes;
// Battery
converted["6061262677a88b001b990ddb"] = decoded.batteryVoltage;
// Box temperature
converted["6061262677a88b001b990dda"] = decoded.boxTemperature;
// Box humidity
converted["6061262677a88b001b990dd9"] = decoded.boxHumidity;
// T1:Air tem
converted["6061262677a88b001b990dd8"] = decoded.T0;
// T2
converted["6061262677a88b001b990dd7"] = decoded.T1;
                                                 
  return converted;
}





//2020.09.12
function Converter(decoded, port) {
  // Merge, split or otherwise
  // mutate decoded fields.
   var converted = {};
  if(decoded.boxid == 'bufo01'){
     	  // Measurement status
  converted["5e81b625f7afec001bbfe726"] = decoded.sessionStatus;
  // Battery
  converted["5e81b625f7afec001bbfe725"] = decoded.batteryVoltage;

// Inside Device Temperature
  converted["5e81b625f7afec001bbfe724"] = decoded.boxTemperature;

// Inside Device Humidity
  converted["5e81b625f7afec001bbfe723"] = decoded.boxHumidity;

// Inside Device Pressure
  converted["5e81b625f7afec001bbfe722"] = decoded.boxPressure;

// T1 Temperature
  converted["5e81b625f7afec001bbfe721"] = decoded.T1;

// T2 Temperature
  converted["5e81b625f7afec001bbfe720"] = decoded.T2;

// T3 Temperature
  converted["5e81b625f7afec001bbfe71f"] = decoded.T3;
// T4 Temperature
  converted["5e865a106feb97001c35e384"] = decoded.T4;
  if(decoded.T4 !== null)
    converted["5e865a106feb97001c35e384"] = decoded.T4;

  if(decoded.T5 !== null)
    converted["5f36f951a4a11c001b0a3795"] = decoded.T5;
  } else {
     if(port == 2){
        converted["5eedf794ee9b25001b289fae"] = decoded.sessionStatus;
       // Battery
        converted["5eedf794ee9b25001b289fad"] = decoded.batteryVoltage;
      // Inside Device Temperature
        converted["5f0cb237987fd4001bbb04a5"] = decoded.boxTemperature;
      // Inside Device Humidity
        converted["5f0cb237987fd4001bbb04a6"] = decoded.boxHumidity;
      // Inside Device Pressure
        converted["5f0cb237987fd4001bbb04a7"] = decoded.boxPressure;

      // T1 Temperature
        converted["5eedf794ee9b25001b289fac"] = decoded.T1;
      // T2 Temperature
        converted["5eedf794ee9b25001b289fab"] = decoded.T2;
      // T3 Temperature
        converted["5f09f31f987fd4001b9f8e28"] = decoded.T3;
      // T4 Temperature

     } else {
       if(port ==3){
       //bufo03
        converted["5f5bf71884e5a2001b6065ed"] = decoded.sessionStatus;
       // Battery
        converted["5f5bf71884e5a2001b6065ec"] = decoded.batteryVoltage;
      // Inside Device Temperature
        converted["5f5bf71884e5a2001b6065e7"] = decoded.boxTemperature;
      // Inside Device Humidity
        converted["5f5bf71884e5a2001b6065e6"] = decoded.boxHumidity;
      // Inside Device Pressure
        converted["5f5bf71884e5a2001b6065e5"] = decoded.boxPressure;

      // T1 Temperature
        converted["5f5bf71884e5a2001b6065eb"] = decoded.T1;
      // T2 Temperature
        converted["5f5bf71884e5a2001b6065ea"] = decoded.T2;
      // T3 Temperature
        converted["5f5bf71884e5a2001b6065e9"] = decoded.T3;
      // T4 Temperature
        converted["5f5bf71884e5a2001b6065e8"] = decoded.T4;
         //return decoded;
             } else {
               return decoded;
             }
       }
  }
  return converted;
}