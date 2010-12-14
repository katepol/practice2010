#ifndef HTMLTOXML_H
#define HTMLTOXML_H

#include "externallibs/include/tidy/tidy.h"
#include "externallibs/include/tidy/buffio.h"
#include <iostream>
#include <errno.h>

static int convertToXml(char const * htmlToConvert, char const * xmlOutput)
{
    TidyBuffer errbuf = {0};
    int rc = -1;

    TidyDoc tdoc = tidyCreate();                     // Initialize "document"
    tidySetCharEncoding(tdoc, "utf8");

    if (tidyOptSetBool(tdoc, TidyXhtmlOut, yes)) // Convert to XHTML
        rc = tidySetErrorBuffer(tdoc, &errbuf);      // Capture diagnostics
    if (rc >= 0)
        rc = tidyParseFile(tdoc, htmlToConvert);           // Parse the input file
    if (rc >= 0)
        rc = tidyCleanAndRepair(tdoc);               // Tidy it up!
    if (rc >= 0)
        rc = tidyRunDiagnostics(tdoc);             // Kvetch
    if (rc > 1)                                    // If error, force output.
        rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
    if (rc >= 0)
        rc = tidySaveFile(tdoc, xmlOutput);          // Pretty Print

    if (rc >= 0)
    {
        //if ( rc > 0 )
            //printf( "\nDiagnostics:\n\n%s", errbuf.bp );
    }
    else
        std::cerr << "A severe error (" << rc << ") occurred. File " << htmlToConvert << "\n";

    tidyBufFree( &errbuf );
    tidyRelease( tdoc );
    return rc;
}
#endif // HTMLTOXML_H
