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
            .vt>tbody>tr>td{
                padding: 2px 20px 2px 20px;	
                border-top-style: solid;
                border-top-width: thin;;

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
            <br>
            <a href="config">Configuration</a><br>
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
    String  out = R"(<select name="address">\n)";
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
    stateTable += getTR("Box name", theDevice.status.nodeName);
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
    stateTable+=R"(</tbody>
	</table>
</div>)";
    return stateTable;
}

String HtmlContent::configInnerBody()
{
    String body;
    body.reserve(2024);

    body+=R"(<div>Configuration
    <form action="save_config">
	<table class="v">	
	    <tbody>)";
    body += getTR("DEVADDR"
        , R"(<input type="text" name=")"
        +String(theDevice.config.DEVADDR_name)
        +R"(" minlength="4" maxlength="8" style="width: 100px;" placeholder="LoRaWAN Device Address" value=")"
        +theDevice.config.getDevAddr()
        +R"(" >)");
    body += getTR("Network Session Key"
        , R"(<input type="text" name=")"
        +String(theDevice.config.NWKSKEY_name)
        +R"(" minlength="32" maxlength="48" style="width: 280px;" placeholder="LoRaWAN NwksKeyF" value=")"
        +theDevice.config.getNwksKey()
        +R"(" >)");
    body += getTR("Application Session Key"
        , R"(<input type="text" name=")"
        +String(theDevice.config.APPSKEY_name)
        +R"(" minlength="32" maxlength="48" style="width: 280px;" placeholder="LoRaWAN AppsKey" value=")"
        +theDevice.config.getAppsKeyStr()
        +R"(" >)");
    body += getTR("LoRaWAN Spreading Factor"
        , R"(<input type="number" name=")"
        +String(theDevice.config.SF_name)
        +R"(" min="7" max="12" style="width: 100px;" placeholder="LoRaWAN SF" value=")"
        +String(theDevice.config.SF)
        +R"(" >)");
    body += getTR("Measurement interval [s]"
        , R"(<input type="number" name="interval" min="10" style="width: 100px;" placeholder="Measuring interval [sec]" value=")"
        +String(theDevice.config.TimeBetween)
        +R"(" > seconds)");
    body += getTR("Number of probes"
        , R"(<input type="number" name="number_of_probes" min="0" style="width: 100px;" placeholder="Number of assigned probes" value=")"
        +String(theDevice.config.lastProbeIndex() +1)
        +R"(" >)");


//    body += getTR("Wakeup reason", theDevice.status.getWUResonStr());
//    body += getTR("#boot", String(theDevice.status.getBootCount()));
//    body += getTR("Temperatures", knownProbesTemperatures());
    body+=R"(</tbody>
	</table>
    <br>
    <input type="submit" value="Save config"/>
    </form>
</div>)";
    return body;


}