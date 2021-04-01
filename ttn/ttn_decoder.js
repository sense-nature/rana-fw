function Decoder(bytes, port) {
	// Decode an uplink message from a buffer to an object of fields
	// bytes: array of uitn8_t, the message data
	// port: uint8_t, the LoRaWAN FPort param
	var boxid = "Rana["+port+"]";
	var sessionStatus = bytes[0];
	var probesStatus = bytes[1];

	var offset = 2;

	var batteryVoltage = ((bytes[offset] << 8) | bytes[offset+1]);
	offset+=2;
	
	var retObject = { port:port, boxid: boxid, sessionStatus: sessionStatus , batteryVoltage: batteryVoltage, probes:0};
	if( ! (sessionStatus & (0x01<<1)) ){
		//there was not BME280 error, read the data
		retObject.boxHumidity = bytes[offset];
		offset+=1;
		var tVal = ((bytes[offset] << 8) | bytes[offset+1]);
		if( tVal > (0xFFFF >> 1) )
	    tVal = -(0xFFFF - tVal);
		retObject.boxTemperature = tVal / 100.0;
		offset+=2;
	}


	for(var i=0; i<8; i++ ){
		if( ! (probesStatus & (0x01<<(i))) ){
			var value = (bytes[offset]<<8) | bytes[offset+1];
			//sign needs to be included in the temperature value
			if( value > (0xFFFF >> 1) )
				value = -(0xFFFF - value);
			retObject["T"+i] = value / 100.0;
			offset+=2;
			retObject["probes"]++;
		}

	}
	return retObject;
}