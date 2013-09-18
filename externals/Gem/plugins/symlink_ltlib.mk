# convenience links: symlink the plugin-binary into top_builddir
# include this makefile snippet in your plugin's Makefile.am to automatically
# create symlinks to all pkglib_LTLIBRARIES binaries into your top builddir


.PHONY: convenience-link clean-convenience-link

convenience-link: $(pkglib_LTLIBRARIES)
	  @for soname in `echo | $(EGREP) "^dlname=" $^ | $(SED) -e "s|^dlname='\(.*\)'|\1|"`; do  \
            echo "$$soname: creating convenience link from $(abs_builddir)/.libs to $(top_builddir)"; \
	    rm -f $(top_builddir)/$$soname ; \
	    test -e $(abs_builddir)/.libs/$$soname && \
	    cd $(top_builddir) && \
	    $(LN_S) $(abs_builddir)/.libs/$$soname $$soname || true;\
	  done 

clean-convenience-link:
	  @for soname in `echo | $(EGREP) "^dlname=" $(pkglib_LTLIBRARIES) | $(SED) -e "s|^dlname='\(.*\)'|\1|"`; do  \
            echo "$$soname: cleaning convenience links"; \
            test -L $(top_builddir)/$$soname && rm -f $(top_builddir)/$$soname || true; \
	  done 
	

all-local:: convenience-link

clean-local:: clean-convenience-link
