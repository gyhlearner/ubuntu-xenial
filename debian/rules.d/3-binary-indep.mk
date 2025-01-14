build-indep:
	@echo Debug: $@

# The binary-indep dependency chain is:
#
# install-headers <- install-doc <- install-source <- install-tools <- install-indep <- binary-indep
# install-headers <- binary-headers
#
indep_hdrpkg = $(indep_hdrs_pkg_name)
indep_hdrdir = $(CURDIR)/debian/$(indep_hdrpkg)/usr/src/$(indep_hdrpkg)
<<<<<<< HEAD
install-headers:
	@echo Debug: $@
	dh_testdir
	dh_testroot
	dh_prep
=======
install-headers: prepare-indep
	@echo Debug: $@
	dh_testdir
	dh_testroot
>>>>>>> temp

ifeq ($(do_flavour_header_package),true)
	install -d $(indep_hdrdir)
	find . -path './debian' -prune -o -path './$(DEBIAN)' -prune \
	  -o -path './include/*' -prune \
	  -o -path './scripts/*' -prune -o -type f \
	  \( -name 'Makefile*' -o -name 'Kconfig*' -o -name 'Kbuild*' -o \
	     -name '*.sh' -o -name '*.pl' -o -name '*.lds' \) \
	  -print | cpio -pd --preserve-modification-time $(indep_hdrdir)
	cp -a scripts include $(indep_hdrdir)
	(find arch -name include -type d -print | \
		xargs -n1 -i: find : -type f) | \
		cpio -pd --preserve-modification-time $(indep_hdrdir)
endif

docpkg = $(doc_pkg_name)
docdir = $(CURDIR)/debian/$(docpkg)/usr/share/doc/$(docpkg)
<<<<<<< HEAD
install-doc: install-headers
=======
install-doc: prepare-indep
>>>>>>> temp
	@echo Debug: $@
ifeq ($(do_doc_package),true)
	dh_testdir
	dh_testroot

	install -d $(docdir)
ifeq ($(do_doc_package_content),true)
	# First the html docs. We skip these for autobuilds
	if [ -z "$(AUTOBUILD)" ]; then \
		install -d $(docdir)/$(doc_pkg_name)-tmp; \
		$(kmake) O=$(docdir)/$(doc_pkg_name)-tmp htmldocs; \
		install -d $(docdir)/html; \
<<<<<<< HEAD
		rsync -aL $(docdir)/$(doc_pkg_name)-tmp/Documentation/DocBook/ \
=======
		rsync -aL $(docdir)/$(doc_pkg_name)-tmp/Documentation/output/ \
>>>>>>> temp
			$(docdir)/html/; \
		rm -rf $(docdir)/$(doc_pkg_name)-tmp; \
	fi
endif
	# Copy the rest
	cp -a Documentation/* $(docdir)
<<<<<<< HEAD
	rm -rf $(docdir)/DocBook
	find $(docdir) -name .gitignore | xargs rm -f
endif

srcpkg = $(src_pkg_name)-source-$(release)
srcdir = $(CURDIR)/debian/$(srcpkg)/usr/src/$(srcpkg)
balldir = $(CURDIR)/debian/$(srcpkg)/usr/src/$(srcpkg)/$(srcpkg)
install-source: install-doc
=======
	find $(docdir) -name .gitignore | xargs rm -f
endif

srcpkg = linux-source-$(release)
srcdir = $(CURDIR)/debian/$(srcpkg)/usr/src/$(srcpkg)
balldir = $(CURDIR)/debian/$(srcpkg)/usr/src/$(srcpkg)/$(srcpkg)
install-source: prepare-indep
>>>>>>> temp
	@echo Debug: $@
ifeq ($(do_source_package),true)

	install -d $(srcdir)
ifeq ($(do_source_package_content),true)
	find . -path './debian' -prune -o -path './$(DEBIAN)' -prune -o \
		-path './.*' -prune -o -print | \
		cpio -pd --preserve-modification-time $(balldir)
	(cd $(srcdir); tar cf - $(srcpkg)) | bzip2 -9c > \
		$(srcdir)/$(srcpkg).tar.bz2
	rm -rf $(balldir)
	find './debian' './$(DEBIAN)' \
		-path './debian/linux-*' -prune -o \
		-path './debian/$(src_pkg_name)-*' -prune -o \
		-path './debian/build' -prune -o \
		-path './debian/files' -prune -o \
		-path './debian/stamps' -prune -o \
		-path './debian/tmp' -prune -o \
		-print | \
		cpio -pd --preserve-modification-time $(srcdir)
	$(LN) $(srcpkg)/$(srcpkg).tar.bz2 $(srcdir)/..
endif
endif

install-tools: toolspkg = $(tools_common_pkg_name)
install-tools: toolsbin = $(CURDIR)/debian/$(toolspkg)/usr/bin
install-tools: toolssbin = $(CURDIR)/debian/$(toolspkg)/usr/sbin
install-tools: toolsman = $(CURDIR)/debian/$(toolspkg)/usr/share/man
<<<<<<< HEAD
=======
install-tools: hosttoolspkg = $(hosttools_pkg_name)
install-tools: hosttoolsbin = $(CURDIR)/debian/$(hosttoolspkg)/usr/bin
install-tools: hosttoolsman = $(CURDIR)/debian/$(hosttoolspkg)/usr/share/man
>>>>>>> temp
install-tools: cloudpkg = $(cloud_common_pkg_name)
install-tools: cloudbin = $(CURDIR)/debian/$(cloudpkg)/usr/bin
install-tools: cloudsbin = $(CURDIR)/debian/$(cloudpkg)/usr/sbin
install-tools: cloudman = $(CURDIR)/debian/$(cloudpkg)/usr/share/man
<<<<<<< HEAD
install-tools: install-source $(stampdir)/stamp-build-perarch
	@echo Debug: $@

ifeq ($(do_tools_common),true)
ifneq ($(DEBIAN),debian.master)
	echo "non-master branch building linux-tools-common, aborting"
	exit 1
endif
=======
install-tools: prepare-indep $(stampdir)/stamp-build-perarch
	@echo Debug: $@

ifeq ($(do_tools_common),true)
>>>>>>> temp
	rm -rf $(builddir)/tools
	install -d $(builddir)/tools
	for i in *; do $(LN) $(CURDIR)/$$i $(builddir)/tools/; done
	rm $(builddir)/tools/tools
	rsync -a tools/ $(builddir)/tools/tools/

	install -d $(toolsbin)
	install -d $(toolsman)/man1

	install -m755 debian/tools/generic $(toolsbin)/usbip
	install -m755 debian/tools/generic $(toolsbin)/usbipd
	install -m644 $(CURDIR)/tools/usb/usbip/doc/*.8 $(toolsman)/man1/

	install -m755 debian/tools/generic $(toolsbin)/cpupower
	install -m644 $(CURDIR)/tools/power/cpupower/man/*.1 $(toolsman)/man1/

	install -m755 debian/tools/generic $(toolsbin)/perf

	install -m755 debian/tools/generic $(toolsbin)/x86_energy_perf_policy
	install -m755 debian/tools/generic $(toolsbin)/turbostat

	cd $(builddir)/tools/tools/perf && make man
	install -m644 $(builddir)/tools/tools/perf/Documentation/*.1 \
		$(toolsman)/man1

	install -d $(toolsman)/man8
	install -m644 $(CURDIR)/tools/power/x86/x86_energy_perf_policy/*.8 $(toolsman)/man8
	install -m644 $(CURDIR)/tools/power/x86/turbostat/*.8 $(toolsman)/man8

<<<<<<< HEAD
=======
ifeq ($(do_cloud_tools),true)
ifeq ($(do_tools_hyperv),true)
>>>>>>> temp
	install -d $(cloudsbin)
	install -m755 debian/tools/generic $(cloudsbin)/hv_kvp_daemon
	install -m755 debian/tools/generic $(cloudsbin)/hv_vss_daemon
	install -m755 debian/tools/generic $(cloudsbin)/hv_fcopy_daemon
	install -m755 debian/tools/generic $(cloudsbin)/lsvmbus
	install -m755 debian/cloud-tools/hv_get_dhcp_info $(cloudsbin)
	install -m755 debian/cloud-tools/hv_get_dns_info $(cloudsbin)
	install -m755 debian/cloud-tools/hv_set_ifconfig $(cloudsbin)

	install -d $(cloudman)/man8
	install -m644 $(CURDIR)/tools/hv/*.8 $(cloudman)/man8
<<<<<<< HEAD

endif

install-indep: install-tools
=======
endif
endif

ifeq ($(do_tools_acpidbg),true)
	install -m755 debian/tools/generic $(toolsbin)/acpidbg
endif

endif

ifeq ($(do_tools_host),true)
	install -d $(hosttoolsbin)
	install -d $(hosttoolsman)/man1

	install -m 755 $(CURDIR)/tools/kvm/kvm_stat/kvm_stat $(hosttoolsbin)/

	cd $(builddir)/tools/tools/kvm/kvm_stat && make man
	install -m644 $(builddir)/tools/tools/kvm/kvm_stat/*.1 \
		$(hosttoolsman)/man1
endif

prepare-indep:
	@echo Debug: $@
	dh_prep -i

install-indep: install-headers install-doc install-source install-tools
>>>>>>> temp
	@echo Debug: $@

# This is just to make it easy to call manually. Normally done in
# binary-indep target during builds.
<<<<<<< HEAD
binary-headers: install-headers
=======
binary-headers: prepare-indep install-headers
>>>>>>> temp
	@echo Debug: $@
	dh_installchangelogs -p$(indep_hdrpkg)
	dh_installdocs -p$(indep_hdrpkg)
	dh_compress -p$(indep_hdrpkg)
	dh_fixperms -p$(indep_hdrpkg)
	dh_installdeb -p$(indep_hdrpkg)
	$(lockme) dh_gencontrol -p$(indep_hdrpkg)
	dh_md5sums -p$(indep_hdrpkg)
	dh_builddeb -p$(indep_hdrpkg)

binary-indep: cloudpkg = $(cloud_common_pkg_name)
binary-indep: install-indep
	@echo Debug: $@
<<<<<<< HEAD

=======
>>>>>>> temp
	dh_installchangelogs -i
	dh_installdocs -i
	dh_compress -i
	dh_fixperms -i
ifeq ($(do_tools_common),true)
<<<<<<< HEAD
ifneq ($(DEBIAN),debian.master)
	echo "non-master branch building linux-cloud-tools-common, aborting"
	exit 1
endif
=======
ifeq ($(do_cloud_tools),true)
ifeq ($(do_tools_hyperv),true)
>>>>>>> temp
	dh_installinit -p$(cloudpkg) -n --name hv-kvp-daemon
	dh_installinit -p$(cloudpkg) -n --name hv-vss-daemon
	dh_installinit -p$(cloudpkg) -n --name hv-fcopy-daemon
	dh_systemd_enable -p$(cloudpkg)
	dh_installinit -p$(cloudpkg) -o --name hv-kvp-daemon
	dh_installinit -p$(cloudpkg) -o --name hv-vss-daemon
	dh_installinit -p$(cloudpkg) -o --name hv-fcopy-daemon
	dh_systemd_start -p$(cloudpkg)
endif
<<<<<<< HEAD
=======
endif
endif
>>>>>>> temp
	dh_installdeb -i
	$(lockme) dh_gencontrol -i
	dh_md5sums -i
	dh_builddeb -i
