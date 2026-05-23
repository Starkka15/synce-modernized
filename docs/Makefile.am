if BUILD_DOCS

html-local: doxygen.stamp

doxygen.stamp: Doxyfile
	$(DOXYGEN) $(DOXYFLAGS) $<
	echo Timestamp > $@

install-html-local:
	$(MKDIR_P) $(DESTDIR)$(docdir)
	cp -R html $(DESTDIR)$(docdir)
	find "$(DESTDIR)$(docdir)/html" -type d -exec chmod 755 {} \;;
	find "$(DESTDIR)$(docdir)/html" ! -type d -exec chmod 644 {} \;;

CLEANFILES = doxywarn.txt doxygen.stamp

clean-local:
	-rm -rf html

endif
