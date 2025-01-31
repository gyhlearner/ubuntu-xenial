# Do udebs if not disabled in the arch-specific makefile
binary-udebs: binary-debs
	@echo Debug: $@
ifeq ($(disable_d_i),)
	@$(MAKE) --no-print-directory -f $(DROOT)/rules DEBIAN=$(DEBIAN) \
		do-binary-udebs
endif

<<<<<<< HEAD
=======
do-binary-udebs: linux_udeb_name=$(shell if echo $(src_pkg_name)|egrep -q '(linux-lts|linux-hwe)'; then echo $(src_pkg_name); else echo linux; fi)
>>>>>>> temp
do-binary-udebs: debian/control
	@echo Debug: $@
	dh_testdir
	dh_testroot

	# unpack the kernels into a temporary directory
	mkdir -p debian/d-i-${arch}

<<<<<<< HEAD
	imagelist=$$(cat $(builddir)/kernel-versions | grep ^${arch} | gawk '{print $$4}') && \
	for i in $$imagelist; do \
	  dpkg -x $$(ls ../linux-image-$$i\_$(release)-$(revision)_${arch}.deb) \
		debian/d-i-${arch}; \
	  if [ -f ../linux-image-extra-$$i\_$(release)-$(revision)_${arch}.deb ] ; then \
	    dpkg -x ../linux-image-extra-$$i\_$(release)-$(revision)_${arch}.deb \
		  debian/d-i-${arch}; \
	  fi; \
=======
	imagelist=$$(cat $(CURDIR)/$(DEBIAN)/d-i/kernel-versions | grep ^${arch} | gawk '{print $$3}') && \
	for f in $$imagelist; do \
	  i=$(release)-$(abinum)-$$f; \
          for f in \
		../linux-image-$$i\_$(release)-$(revision)_${arch}.deb \
		../linux-image-unsigned-$$i\_$(release)-$(revision)_${arch}.deb \
		../linux-modules-$$i\_$(release)-$(revision)_${arch}.deb \
		../linux-modules-extra-$$i\_$(release)-$(revision)_${arch}.deb; \
	  do \
		  [ -f $$f ] && dpkg -x $$f debian/d-i-${arch}; \
	  done; \
>>>>>>> temp
	  /sbin/depmod -b debian/d-i-${arch} $$i; \
	done

	# kernel-wedge will error if no modules unless this is touched
<<<<<<< HEAD
	touch $(CURDIR)/debian/build/no-modules

	touch ignore-dups
	export SOURCEDIR=$(CURDIR)/debian/d-i-${arch} && \
	  cd $(builddir) && \
	  kernel-wedge install-files && \
=======
	touch $(DEBIAN)/d-i/no-modules

	touch $(CURDIR)/$(DEBIAN)/d-i/ignore-dups
	export KW_DEFCONFIG_DIR=$(CURDIR)/$(DEBIAN)/d-i && \
	export KW_CONFIG_DIR=$(CURDIR)/$(DEBIAN)/d-i && \
	export SOURCEDIR=$(CURDIR)/debian/d-i-${arch} && \
	  kernel-wedge install-files $(release)-$(abinum) && \
>>>>>>> temp
	  kernel-wedge check

        # Build just the udebs
	dilist=$$(dh_listpackages -s | grep "\-di$$") && \
	[ -z "$dilist" ] || \
	for i in $$dilist; do \
	  dh_fixperms -p$$i; \
	  $(lockme) dh_gencontrol -p$$i; \
	  dh_builddeb -p$$i; \
	done
	
	# Generate the meta-udeb dependancy lists.
	@gawk '										\
		/^Package:/ {								\
			package=$$2; flavour=""; parch="" }				\
<<<<<<< HEAD
		(/Package-Type: udeb/ && package !~ /^linux-udebs-/) {      \
=======
		(/Package-Type: udeb/ && package !~ /^$(linux_udeb_name)-udebs-/) {      \
>>>>>>> temp
			match(package, "'$(release)'-'$(abinum)'-(.*)-di", bits);       \
			flavour = bits[1];						\
		}									\
		(/^Architecture:/ && $$0 " " ~ / '$(arch)'/) {				\
			parch=$$0;							\
		}									\
		(flavour != "" && parch != "") {					\
			udebs[flavour] = udebs[flavour] package ", ";			\
			flavour=""; parch="";						\
		}                                                      			\
		END {                                                  			\
			for (flavour in udebs) {					\
<<<<<<< HEAD
				package="linux-udebs-" flavour;		\
=======
				package="$(linux_udeb_name)-udebs-" flavour;		\
>>>>>>> temp
				file="debian/" package ".substvars";			\
				print("udeb:Depends=" udebs[flavour]) > file;		\
				metas="'$(builddir)'/udeb-meta-packages";		\
				print(package) >metas					\
			}								\
		}									\
	' <$(CURDIR)/debian/control
	@while read i; do \
		$(lockme) dh_gencontrol -p$$i; \
		dh_builddeb -p$$i; \
	done <$(builddir)/udeb-meta-packages
