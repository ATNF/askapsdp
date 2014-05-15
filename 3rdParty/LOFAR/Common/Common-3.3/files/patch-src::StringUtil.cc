--- src/StringUtil.cc.orig	2013-10-31 11:11:11.493249000 +0800
+++ src/StringUtil.cc	2013-10-31 11:12:01.192950000 +0800
@@ -95,7 +95,7 @@
 //
 const string timeString(time_t		aTime, 
 							 bool		gmt,
-							 char* 		format)
+							 const char* 		format)
 {
 	char	theTimeString [256];
 	strftime(theTimeString, 256, format, gmt ? gmtime(&aTime) 
@@ -134,7 +134,7 @@
 //
 // NOTE: original string is truncated!
 //
-int32 rtrim(char*	aCstring, int32		len, char*	whiteSpace)
+int32 rtrim(char*	aCstring, int32		len, const char*	whiteSpace)
 {
 	if (!aCstring || !(*aCstring)) {		// aCstring must be valid
 		return (0);
@@ -157,7 +157,7 @@
 //
 // skip leading spaces. A pointer to the first non-whitespace char is
 // returned.
-char*	ltrim(char*	aCstring, char*	whiteSpace)
+char*	ltrim(char*	aCstring, const char*	whiteSpace)
 {
 	aCstring += strspn(aCstring, whiteSpace);
 
