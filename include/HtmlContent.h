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
        
    protected:
        String knownProbesTemperatures();
        String unknownProbesDropdown();
        int lastProbeIndex();


    };
    
    
   
    
    

}//Rana


#endif // __HTLMCONTENT_H__