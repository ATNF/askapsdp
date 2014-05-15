--- fits/FITS/FITSDateUtil.cc.orig	2014-01-28 17:24:52.000000000 +1100
+++ fits/FITS/FITSDateUtil.cc	2014-01-28 17:25:18.000000000 +1100
@@ -60,7 +60,7 @@
 	    ostringstream out;
 	    out << setfill('0') << setw(2) << day << "/" << setw(2) << month <<
 		"/" << setw(2) << year;
-	    date = out;
+	    date = out.str();
 	}
 	break;
     case NEW_DATEONLY:
