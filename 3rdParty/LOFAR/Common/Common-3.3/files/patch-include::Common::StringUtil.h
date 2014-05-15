--- include/Common/StringUtil.h.orig	2013-10-31 11:09:17.571509000 +0800
+++ include/Common/StringUtil.h	2013-10-31 11:10:23.692454000 +0800
@@ -111,7 +111,7 @@
 // format. The default format is yyyy-mm-dd hh:mm:ss
 const string timeString(time_t     aTime, 
 						bool       gmt = true,
-						char*      format = "%F %T");
+						const char*      format = "%F %T");
 
 // Skip leading whitespace (blanks and horizontal tabs) starting at st.
 // It returns the position of the first non-whitespace character.
@@ -150,14 +150,14 @@
 //
 // NOTE: original string is truncated!
 //
-int32 	rtrim(char*	aCstring, int32		len = 0, char* whiteSpace = " 	");
+int32 	rtrim(char*	aCstring, int32		len = 0, const char* whiteSpace = " 	");
 
 //
 // char* ltrim(char*	Cstring)
 //
 // Skip leading spaces. A pointer to the first non-whitespace char is
 // returned. (points into original string)
-char*	ltrim(char*	aCstring, char* whiteSpace = " 	");
+char*	ltrim(char*	aCstring, const char* whiteSpace = " 	");
 
 //
 // rtrim(aString)
