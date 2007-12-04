all:
	@echo "OK"
install:
	@if [ ! -d ${DESTDIR}/usr/lib/xlyrics/HTTP ] ; then \
	mkdir -p ${DESTDIR}/usr/lib/xlyrics/HTTP ;\
	fi
	@cp Lite.pm ${DESTDIR}/usr/lib/xlyrics/HTTP/
	@cp downloadlyrics.pl ${DESTDIR}/usr/lib/xlyrics/
	@chmod 755 ${DESTDIR}/usr/lib/xlyrics/HTTP/Lite.pm
	@chmod 755 ${DESTDIR}/usr/lib/xlyrics/downloadlyrics.pl
uninstall:
	@rm -rf /usr/lib/xlyrics/HTTP
	@rm -f /usr/lib/xlyrics/downloadlyrics.pl
clean:
	@echo "OK"
distclean:
	@echo "OK"
