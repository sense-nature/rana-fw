#include "HtmlContent.h"




using namespace Rana;

const char * TRs = "<tr>";
const char * TRe = "</tr>";
const char * TDs = "<td>";
const char * TDe = "</td>";


HtmlContent::HtmlContent(Device & dev):theDevice(dev) 
{}

HtmlContent::~HtmlContent() 
{}


const char * HtmlContent::getPageTop() const
{
    static const char top[] = R"(<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="utf-8" />
        <title>RanaBox WebUI, current box state</title>
        <style>
            th {color:#555555;}
            table {
                /*border: 3px double green; */
            
            }
            td {
                /*border: 1px solid red;  
                margin-right: 5px; */
                border-collapse: collapse;
                vertical-align:top;
                padding: 5px 20px 2px 2px;
                
            }
            .v>tbody>tr:nth-child(odd) {
                background:#efefef
            }
            .v>tbody>tr:nth-child(even) {
                background:#ffffff
            }
            
            .r {
                text-align:right;
            }
            .l {
                text-align:left;
            }
            .vt{
                border-spacing: 1px 0;	
            }	
            .conf_div{
                background:#f8f8f8;
                border-style: solid;
                border-width: 1px;
                margin: 10px;
                padding: 10px;
            }
            .conf_title{
                font-weight: bold;
            }

            .vt>tbody>tr>td{
                padding: 15px 20px 15px 20px;	
                border-top-style: solid;
                border-top-width: thin;
            }
        </style>
    </head>
    <body>
)";
    return top;     
}

const char * HtmlContent::getPageFooter() const
{
    static const char footer [] = R"(
        <div class="footer">
            <hr>
            <a href="state">Current box state</a>
            &nbsp;&nbsp;&nbsp;&nbsp;
            <a href="config">Configuration</a>
            &nbsp;&nbsp;&nbsp;&nbsp;
            <a href="restart">Restart node</a><br>
        </div>
    </body>
</html>)";
    return footer;    
}


String getTR(const String &firstCell, const String &secondCell)
{
    return String("<tr><td>")+firstCell+"</td><td>"+ secondCell+"</td></tr>\n";
}


String HtmlContent::unknownProbesDropdown()
{
    String  out = R"(<select name="address">
)";
    for(auto it = theDevice.status.unknownProbeTemperatures.begin(); it!=theDevice.status.unknownProbeTemperatures.end(); it++){
        String addr = devAddrToString(it->first);
        String option = R"(<option value="{1}">{1}</option>
)";
        option.replace("{1}", addr);
        out += option;
    }
    out+=R"(</select>)";
    return out;

}

String HtmlContent::knownProbesTemperatures()
{
    String table;
    table.reserve(1024);
    table += R"(
        <table class="v vt">
			<thead>
				<th>T#</th>
				<th>address</th>
				<th>current<br>temperature</th>
			</thead>
			<tbody>)";
    for(auto it = theDevice.config.Probes.begin(); it != theDevice.config.Probes.end(); it++ ){
        String s = R"(<tr><td>T{1}</td><td>{2}</td><td>{3}</td></tr>
        )";
        s.replace("{1}",String(it->first));
        s.replace("{2}",devAddrToString(it->second) );

        auto i2 = theDevice.status.knownProbeTemperatures.find(it->first);
        if( i2 != theDevice.status.knownProbeTemperatures.end() ){
            s.replace("{3}",String(i2->second.second)+"&deg;C");
        } else {
            if(theDevice.status.unknownProbeTemperatures.empty()){
                s.replace("{3}","--");
            } else {
                String assignForm = R"(--  <form action="assign">
    <input type="hidden" name="T" value="{1}">
    {S}
    <input type="submit" value="Assign">
</form>)";
                assignForm.replace("{1}",String(it->first));
                assignForm.replace("{S}", unknownProbesDropdown());
                s.replace("{3}",assignForm);
            }
        }
        table += s;
    }

    table += R"(
            </tbody>		
		</table>)";

    return table;

}


String HtmlContent::formSetTimeFromBrowser() const
{
    String sForm = R"(
    	<form name="time_from_browser" action="time" onsubmit="{timestamp.value = Math.round(Date.now()/1000.0); return true;}" >
				<input type="hidden" name="timestamp" value="" />
				<button type="submit" name="source" value="browser"  >Time from the web browser</button>				
				<button type="submit" name="source" value=""  >Check current time</button>				
			</form>   
)";
    return sForm;       
}

String HtmlContent::formSetNextMeasurment() const
{
    String sForm = R"(
    	<form name="next_measurement" action="next_measurement" >
				<input type="number" name="next_measurement" min="1" style="width: 100px;" value=")"
        +String(theDevice.status.measurementCount+1)
        +R"(" > 
                <input type="submit" value="Set value">
			</form>   
)";
    return sForm;       
}

String HtmlContent::configDivRTC()
{
     String divStr;
    divStr.reserve(2024);

    divStr+=R"(
    <div class="conf_div">
        <div class="conf_title">External RTC + Time</div>
    <br>
   <table class="v vt">
        <tbody>
)";
    divStr+= getTR("Local time", theDevice.status.rtcTimeStr() + formSetTimeFromBrowser());        
    divStr+= getTR("Next measurement", formSetNextMeasurment() );
    divStr+=R"( 
        </tbody>
    </table>
    </div>
    )";
    return divStr;
}




String HtmlContent::currentStateInnerBody()
{
    String stateTable;
    stateTable.reserve(2024);

    stateTable+=R"(<div>
	<table class="v">	
	    <thead>
	        <tr>
	            <th>item</th>
	            <th>value</th>	
            </tr>	
	    </thead>
	    <tbody>)";
    stateTable += getTR("Node name", theDevice.config.NodeName);
    stateTable += getTR("MAC address", theDevice.GetHWString());
    int battPercent = (int)(((double)theDevice.status.batteryLevel - 1700.0) / 5.2);
    stateTable += getTR("Battery", String(theDevice.status.batteryLevel) +" &nbsp;(~"+String(battPercent)+"%)");
    stateTable += getTR("Local time", theDevice.status.rtcTimeStr());
    stateTable += getTR("Box climate", theDevice.status.getInternalSensorValues());
    stateTable += getTR("Measure interval", 
        String(theDevice.config.TimeBetween)
        +" s  ("
        + String(theDevice.config.TimeBetween / 60)
        +" min "
        +String(theDevice.config.TimeBetween % 60)
        +" s )" );
    stateTable += getTR("Wakeup reason", theDevice.status.getWUResonStr());
    stateTable += getTR("#boot", String(theDevice.status.getBootCount()));
    stateTable += getTR("#measurement", String(theDevice.status.measurementCount));
    stateTable += getTR("Temperatures", knownProbesTemperatures());
    stateTable += getTR("RAM", String());
    stateTable+=R"(</tbody>
	</table>
</div>)";
    return stateTable;
}


String HtmlContent::configDivGeneral()
{
    String divStr;
    divStr.reserve(2024);

    divStr+=R"(<div class="conf_div">
    <div class="conf_title">Configuration </div> 
    <br>
    <form action="save_config">
	<table class="v vt">	
	    <tbody>)";
    divStr += getTR("Measurement interval [s]"
        , R"(<input type="number" name="interval" min="10" style="width: 100px;" placeholder="Measuring interval [sec]" value=")"
        +String(theDevice.config.TimeBetween)
        +R"(" > seconds)");
    divStr+=R"(</tbody>
	</table>
    <br>
    <input type="submit" value="Save config"/>
    </form>
</div>
)";
    return divStr;

}

String HtmlContent::configDivProbeAssignment()
{
 String divStr;
    divStr.reserve(2024);

    divStr+=R"(<div class="conf_div">
     <div class="conf_title">Probes address assignment</div> 
     <br>
)";

    String table;
    table.reserve(1024);
    table += R"(
        <table class="v vt">
			<thead>
                <tr>
				<th>T#</th>
				<th>address</th>
				<th>current<br>temperature</th>
                </tr>
			</thead>
			<tbody>)";

        const String releaseForm = R"(<form action="assign">
            <input type="hidden" name="T" value="{1}">
            <input type="hidden" name="address" value="Release">
            <input type="submit" value="Release address">
        </form>
        )";
        const String assignForm = R"(<form action="assign">
    <input type="hidden" name="T" value="{1}">
    {S}
    <input type="submit" value="Assign">
</form>)";

    uint8_t lastIndex = 7;
    for(auto it = theDevice.config.Probes.begin(); it != theDevice.config.Probes.end(); it++ )
        if(lastIndex < it->first)
            lastIndex = it->first;
    for(uint8_t index = 0; index<=lastIndex; index++){
        String strAddreess = "NOT ASSIGNED";
        if(theDevice.config.Probes.count(index) >0 )
            strAddreess = devAddrToString(theDevice.config.Probes[index]);
//    for(auto it = theDevice.config.Probes.begin(); it != theDevice.config.Probes.end(); it++ ){
        String s = R"(<tr><td>T{1}</td><td>{2}</td><td>{3}</td></tr>
)";
        auto i2 = theDevice.status.knownProbeTemperatures.find(index);
        if( i2 != theDevice.status.knownProbeTemperatures.end() ){
            s.replace("{3}",String(i2->second.second)+" &deg;C " + releaseForm);       
        } else {
            if(theDevice.status.unknownProbeTemperatures.empty()){
                s.replace("{3}","NOT FOUND!!! " + releaseForm);
            } else {
                s.replace("{3}",assignForm);
                s.replace("{S}",unknownProbesDropdown());
            }
        }
        s.replace("{1}",String(index));
        s.replace("{2}",strAddreess);
        table += s;
    }
    table += R"(
            </tbody>		
		</table>)";
    divStr += table;

    divStr+=R"(
</div>
)";
    return divStr;
} 


String HtmlContent::configDivNameLoRaWAN()
{
    String divStr;
    divStr.reserve(2024);

    divStr+=R"(<div class="conf_div"> 
    <div class="conf_title">Identification + LoRaWAN</div>
    <br>
    <form action="save_config">
	<table class="v vt">	
	    <tbody>)";
    divStr += getTR("Node name <br><small>(not used by LoRaWAN)</small>"
        , R"(<input type="text" name=")"
        +String(theDevice.config.NodeName_name)
        +R"(" minlength="3" maxlength="32" style="width: 200px;" placeholder="Node name" value=")"
        +theDevice.config.NodeName
        +R"(" >)");        
    divStr += getTR("DEVADDR"
        , R"(<input type="text" name=")"
        +String(theDevice.config.DEVADDR_name)
        +R"(" minlength="4" maxlength="8" style="width: 100px;" placeholder="LoRaWAN Device Address" value=")"
        +theDevice.config.getDevAddr()
        +R"(" >)");
    divStr += getTR("Network Session Key"
        , R"(<input type="text" name=")"
        +String(theDevice.config.NWKSKEY_name)
        +R"(" minlength="32" maxlength="48" style="width: 280px;" placeholder="LoRaWAN NwksKeyF" value=")"
        +theDevice.config.getNwksKey()
        +R"(" >)");
    divStr += getTR("Application Session Key"
        , R"(<input type="text" name=")"
        +String(theDevice.config.APPSKEY_name)
        +R"(" minlength="32" maxlength="48" style="width: 280px;" placeholder="LoRaWAN AppsKey" value=")"
        +theDevice.config.getAppsKeyStr()
        +R"(" >)");
    divStr += getTR("LoRaWAN Spreading Factor"
        , R"(<input type="number" name=")"
        +String(theDevice.config.SF_name)
        +R"(" min="7" max="12" style="width: 100px;" placeholder="LoRaWAN SF" value=")"
        +String(theDevice.config.SF)
        +R"(" >)");
    divStr += getTR("Measurement interval [s]"
        , R"(<input type="number" name="interval" min="10" style="width: 100px;" placeholder="Measuring interval [sec]" value=")"
        +String(theDevice.config.TimeBetween)
        +R"(" > seconds)");
    divStr+=R"(</tbody>
	</table>
    <br>
    <input type="submit" value="Save Ident + LoRaWAN"/>
    </form>
</div>
)";
    return divStr;
}


String HtmlContent::configInnerBody()
{
    const char * brrr = "<br><br>\n";
    String body;
    body.reserve(4*1024);
    body += configDivRTC();
    body += brrr;
    body += configDivGeneral();
    body += brrr;
    body += configDivProbeAssignment();
    body += brrr;
    body += configDivNameLoRaWAN();
    body += brrr;
    return body;
}