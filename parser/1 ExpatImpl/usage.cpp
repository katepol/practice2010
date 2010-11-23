bool ParseSomeXML (char const * pszFileName)
{
	// Create the parser 
	MyParser sParser;

	if (!sParser .Create ())
		return false;
	
	FILE *fp = fopen (pszFileName, "r");
	if (fp == NULL)
		return false;
	
	// Loop while there is data
	bool fSuccess = true;
	while (!feof (fp) && fSuccess)
	{
		char* pszBuffer = (char*)sParser.GetBuffer (256); // REQUEST

		if (pszBuffer == NULL)
			fSuccess = false;
		else
		{
			int nLength = fread (pszBuffer, 1, 256, fp); // READ
			fSuccess = sParser .ParseBuffer (nLength, nLength == 0); // PARSE
		}	
	}
	fclose (fp);
	return fSuccess;
}
