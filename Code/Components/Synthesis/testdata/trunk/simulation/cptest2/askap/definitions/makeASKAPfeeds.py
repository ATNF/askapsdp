
nfeeds=32

print "feeds.spacing=0.5deg"
feeds="[feed0"
for feed in range(1,4*nfeeds-1):
    feeds="%s, feed%d"%(feeds,feed)
feeds="%s, feed%d]"%(feeds,4*nfeeds-1)

print "feeds.names=%s"%feeds

feedid=0
for cenx in [-0.5,+0.5]:
    for ceny in [-0.5,+0.5]:
    
        for feed in range(nfeeds):
            if feed<4:
                x=feed+1
                y=0
            elif feed<28:
                x=(feed+2)%6
                y=(feed+2-x)/6
            else:
                x=feed-28+1
                y=5
            fx=2*x+cenx-5.0
            fy=2*y+ceny-5.0
            feedid=feedid+1
            print "feeds.feed%d=[%f,%f]"%(feedid,fx,fy)

