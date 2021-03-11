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
            s.replace("{3}","--");
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
    stateTable += getTR("Wakeup reason", theDevice.status.getWUResonStr());
    stateTable += getTR("#boot", String(theDevice.status.getBootCount()));
    stateTable += getTR("#measurement", String(theDevice.status.measurementCount));

    stateTable += getTR("Temperatures", knownProbesTemperatures());
    

    stateTable+=R"(</tbody>
	</table>
</div>)";
    return stateTable;
}
