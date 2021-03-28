#ifndef __HTLMCONTENT_H__
#define __HTLMCONTENT_H__

#include "RanaDevice.h" 


namespace Rana
{
    class HtmlContent
    {
    private:
        Device &theDevice;
    public:
        HtmlContent( Device &dev);
        ~HtmlContent() ;

        const char * getPageTop() const;
        const char * getPageFooter() const;

        String currentStateInnerBody();
        String configInnerBody();


        String formSetTimeFromBrowser() const;
        String formSetNextMeasurment() const;

    protected:
        String configDivNameLoRaWAN();
        String configDivGeneral();
        String configDivProbeAssignment(); 
        String configDivRTC();


        String knownProbesTemperatures();
        String unknownProbesDropdown();

    };
    
    
   
    
    

}//Rana


#endif // __HTLMCONTENT_H__