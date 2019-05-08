
gen = generator(protoc,
	output: ['@BASENAME@.frigg_pb.hpp'],
	arguments: [
		'--plugin=protoc-gen-frigg=' + frigg_pb.path(),
		'--frigg_out=@BUILD_DIR@',
		'@EXTRA_ARGS@',
		'@INPUT@']) 

pb_sources = [
	gen.process(
			meson.current_source_dir() + '/../../bragi/proto/fs.proto',
			meson.current_source_dir() + '/../../bragi/proto/mbus.proto',
			meson.current_source_dir() + '/../../bragi/proto/posix.proto',
		extra_args: ['--proto_path=' + meson.current_source_dir() + '/../../bragi/proto']),
	gen.process(
			meson.current_source_dir() + '/../../protocols/clock/clock.proto',
		extra_args: ['--proto_path=' + meson.current_source_dir() + '/../../protocols/clock']),
	gen.process(
			meson.current_source_dir() + '/../../protocols/hw/hw.proto',
		extra_args: ['--proto_path=' + meson.current_source_dir() + '/../../protocols/hw']),
	gen.process(
			meson.current_source_dir() + '/../../protocols/kernlet/kernlet.proto',
		extra_args: ['--proto_path=' + meson.current_source_dir() + '/../../protocols/kernlet']),
	gen.process(
			meson.current_source_dir() + '/../../protocols/svrctl/svrctl.proto',
		extra_args: ['--proto_path=' + meson.current_source_dir() + '/../../protocols/svrctl']),
]

thor_sources = files(
	'../../frigg/src/debug.cpp',
	'../../frigg/src/libc.cpp',
	'../../frigg/src/arch_x86/gdt.cpp',
	'../../frigg/src/arch_x86/idt.cpp',
	'../../frigg/src/arch_x86/tss.cpp',
	'../common/font-8x16.cpp',
	'arch/x86/early_stubs.S',
	'arch/x86/stubs.S',
	'arch/x86/cpu.cpp',
	'arch/x86/entry.S',
	'arch/x86/ints.cpp',
	'arch/x86/pic.cpp',
	'arch/x86/system.cpp',
	'arch/x86/paging.cpp',
	'arch/x86/hpet.cpp',
	'arch/x86/rtc.cpp',
	'generic/physical.cpp',
	'generic/main.cpp',
	'generic/service.cpp',
	'generic/hel.cpp',
	'generic/cancel.cpp',
	'generic/core.cpp',
	'generic/fiber.cpp',
	'generic/ipc-queue.cpp',
	'generic/usermem.cpp',
	'generic/schedule.cpp',
	'generic/futex.cpp',
	'generic/stream.cpp',
	'generic/timer.cpp',
	'generic/thread.cpp',
	'generic/event.cpp',
	'generic/irq.cpp',
	'generic/io.cpp',
	'generic/kernlet.cpp',
	'generic/servers.cpp',
	'generic/service_helpers.cpp',
	'generic/work-queue.cpp',
	'system/boot-screen.cpp',
	'system/fb.cpp',
	'system/pci/pci_io.cpp',
	'system/pci/pci_discover.cpp',
	'system/acpi/glue.cpp',
	'system/acpi/madt.cpp',
	'system/acpi/pm-interface.cpp')

acpica_sources = files(
	'../../subprojects/acpica/source/components/events/evgpeinit.c',
	'../../subprojects/acpica/source/components/events/evgpe.c',
	'../../subprojects/acpica/source/components/events/evgpeblk.c',
	'../../subprojects/acpica/source/components/events/evgpeutil.c',
	'../../subprojects/acpica/source/components/events/evglock.c',
	'../../subprojects/acpica/source/components/events/evregion.c',
	'../../subprojects/acpica/source/components/events/evrgnini.c',
	'../../subprojects/acpica/source/components/events/evhandler.c',
	'../../subprojects/acpica/source/components/events/evmisc.c',
	'../../subprojects/acpica/source/components/events/evevent.c',
	'../../subprojects/acpica/source/components/events/evsci.c',
	'../../subprojects/acpica/source/components/events/evxface.c',
	'../../subprojects/acpica/source/components/events/evxfevnt.c',
	'../../subprojects/acpica/source/components/events/evxfregn.c',
	'../../subprojects/acpica/source/components/dispatcher/dsargs.c',
	'../../subprojects/acpica/source/components/dispatcher/dscontrol.c',
	'../../subprojects/acpica/source/components/dispatcher/dsutils.c',
	'../../subprojects/acpica/source/components/dispatcher/dsinit.c',
	'../../subprojects/acpica/source/components/dispatcher/dsfield.c',
	'../../subprojects/acpica/source/components/dispatcher/dsobject.c',
	'../../subprojects/acpica/source/components/dispatcher/dsopcode.c',
	'../../subprojects/acpica/source/components/dispatcher/dsdebug.c',
	'../../subprojects/acpica/source/components/dispatcher/dsmethod.c',
	'../../subprojects/acpica/source/components/dispatcher/dswload.c',
	'../../subprojects/acpica/source/components/dispatcher/dswload2.c',
	'../../subprojects/acpica/source/components/dispatcher/dswstate.c',
	'../../subprojects/acpica/source/components/dispatcher/dswexec.c',
	'../../subprojects/acpica/source/components/dispatcher/dswscope.c',
	'../../subprojects/acpica/source/components/dispatcher/dsmthdat.c',
	'../../subprojects/acpica/source/components/executer/exdebug.c',
	'../../subprojects/acpica/source/components/executer/exconfig.c',
	'../../subprojects/acpica/source/components/executer/exconvrt.c',
	'../../subprojects/acpica/source/components/executer/exconcat.c',
	'../../subprojects/acpica/source/components/executer/excreate.c',
	'../../subprojects/acpica/source/components/executer/exfield.c',
	'../../subprojects/acpica/source/components/executer/exfldio.c',
	'../../subprojects/acpica/source/components/executer/exmutex.c',
	'../../subprojects/acpica/source/components/executer/exnames.c',
	'../../subprojects/acpica/source/components/executer/exutils.c',
	'../../subprojects/acpica/source/components/executer/exresnte.c',
	'../../subprojects/acpica/source/components/executer/extrace.c',
	'../../subprojects/acpica/source/components/executer/exmisc.c',
	'../../subprojects/acpica/source/components/executer/exregion.c',
	'../../subprojects/acpica/source/components/executer/exoparg1.c',
	'../../subprojects/acpica/source/components/executer/exoparg2.c',
	'../../subprojects/acpica/source/components/executer/exoparg3.c',
	'../../subprojects/acpica/source/components/executer/exoparg6.c',
	'../../subprojects/acpica/source/components/executer/exstore.c',
	'../../subprojects/acpica/source/components/executer/exstoren.c',
	'../../subprojects/acpica/source/components/executer/exstorob.c',
	'../../subprojects/acpica/source/components/executer/exsystem.c',
	'../../subprojects/acpica/source/components/executer/exresop.c',
	'../../subprojects/acpica/source/components/executer/exresolv.c',
	'../../subprojects/acpica/source/components/executer/exprep.c',
	'../../subprojects/acpica/source/components/hardware/hwacpi.c',
	'../../subprojects/acpica/source/components/hardware/hwesleep.c',
	'../../subprojects/acpica/source/components/hardware/hwregs.c',
	'../../subprojects/acpica/source/components/hardware/hwvalid.c',
	'../../subprojects/acpica/source/components/hardware/hwgpe.c',
	'../../subprojects/acpica/source/components/hardware/hwpci.c',
	'../../subprojects/acpica/source/components/hardware/hwsleep.c',
	'../../subprojects/acpica/source/components/hardware/hwxface.c',
	'../../subprojects/acpica/source/components/hardware/hwxfsleep.c',
	'../../subprojects/acpica/source/components/namespace/nsalloc.c',
	'../../subprojects/acpica/source/components/namespace/nsaccess.c',
	'../../subprojects/acpica/source/components/namespace/nsarguments.c',
	'../../subprojects/acpica/source/components/namespace/nsconvert.c',
	'../../subprojects/acpica/source/components/namespace/nseval.c',
	'../../subprojects/acpica/source/components/namespace/nsload.c',
	'../../subprojects/acpica/source/components/namespace/nsinit.c',
	'../../subprojects/acpica/source/components/namespace/nsobject.c',
	'../../subprojects/acpica/source/components/namespace/nsnames.c',
	'../../subprojects/acpica/source/components/namespace/nsrepair.c',
	'../../subprojects/acpica/source/components/namespace/nsrepair2.c',
	'../../subprojects/acpica/source/components/namespace/nssearch.c',
	'../../subprojects/acpica/source/components/namespace/nsxfeval.c',
	'../../subprojects/acpica/source/components/namespace/nsxfname.c',
	'../../subprojects/acpica/source/components/namespace/nsxfobj.c',
	'../../subprojects/acpica/source/components/namespace/nsutils.c',
	'../../subprojects/acpica/source/components/namespace/nswalk.c',
	'../../subprojects/acpica/source/components/namespace/nsparse.c',
	'../../subprojects/acpica/source/components/namespace/nspredef.c',
	'../../subprojects/acpica/source/components/namespace/nsprepkg.c',
	'../../subprojects/acpica/source/components/parser/psargs.c',
	'../../subprojects/acpica/source/components/parser/psparse.c',
	'../../subprojects/acpica/source/components/parser/psloop.c',
	'../../subprojects/acpica/source/components/parser/psobject.c',
	'../../subprojects/acpica/source/components/parser/psopcode.c',
	'../../subprojects/acpica/source/components/parser/psopinfo.c',
	'../../subprojects/acpica/source/components/parser/psscope.c',
	'../../subprojects/acpica/source/components/parser/psutils.c',
	'../../subprojects/acpica/source/components/parser/pstree.c',
	'../../subprojects/acpica/source/components/parser/pswalk.c',
	'../../subprojects/acpica/source/components/parser/psxface.c',
	'../../subprojects/acpica/source/components/resources/rscreate.c',
	'../../subprojects/acpica/source/components/resources/rscalc.c',
	'../../subprojects/acpica/source/components/resources/rsxface.c',
	'../../subprojects/acpica/source/components/resources/rsinfo.c',
	'../../subprojects/acpica/source/components/resources/rsaddr.c',
	'../../subprojects/acpica/source/components/resources/rsirq.c',
	'../../subprojects/acpica/source/components/resources/rsio.c',
	'../../subprojects/acpica/source/components/resources/rsserial.c',
	'../../subprojects/acpica/source/components/resources/rsmisc.c',
	'../../subprojects/acpica/source/components/resources/rslist.c',
	'../../subprojects/acpica/source/components/resources/rsmemory.c',
	'../../subprojects/acpica/source/components/resources/rsutils.c',
	'../../subprojects/acpica/source/components/tables/tbdata.c',
	'../../subprojects/acpica/source/components/tables/tbfadt.c',
	'../../subprojects/acpica/source/components/tables/tbfind.c',
	'../../subprojects/acpica/source/components/tables/tbinstal.c',
	'../../subprojects/acpica/source/components/tables/tbxface.c',
	'../../subprojects/acpica/source/components/tables/tbxfload.c',
	'../../subprojects/acpica/source/components/tables/tbxfroot.c',
	'../../subprojects/acpica/source/components/tables/tbutils.c',
	'../../subprojects/acpica/source/components/tables/tbprint.c',
	'../../subprojects/acpica/source/components/utilities/utaddress.c',
	'../../subprojects/acpica/source/components/utilities/utalloc.c',
	'../../subprojects/acpica/source/components/utilities/utascii.c',
	'../../subprojects/acpica/source/components/utilities/utbuffer.c',
	'../../subprojects/acpica/source/components/utilities/utcache.c',
	'../../subprojects/acpica/source/components/utilities/utdecode.c',
	'../../subprojects/acpica/source/components/utilities/utcopy.c',
	'../../subprojects/acpica/source/components/utilities/utmath.c',
	'../../subprojects/acpica/source/components/utilities/utstrtoul64.c',
	'../../subprojects/acpica/source/components/utilities/utdelete.c',
	'../../subprojects/acpica/source/components/utilities/uteval.c',
	'../../subprojects/acpica/source/components/utilities/utexcep.c',
	'../../subprojects/acpica/source/components/utilities/uterror.c',
	'../../subprojects/acpica/source/components/utilities/uthex.c',
	'../../subprojects/acpica/source/components/utilities/utids.c',
	'../../subprojects/acpica/source/components/utilities/utinit.c',
	'../../subprojects/acpica/source/components/utilities/utglobal.c',
	'../../subprojects/acpica/source/components/utilities/utstring.c',
	'../../subprojects/acpica/source/components/utilities/utobject.c',
	'../../subprojects/acpica/source/components/utilities/utownerid.c',
	'../../subprojects/acpica/source/components/utilities/utlock.c',
	'../../subprojects/acpica/source/components/utilities/utmutex.c',
	'../../subprojects/acpica/source/components/utilities/utmisc.c',
	'../../subprojects/acpica/source/components/utilities/utpredef.c',
	'../../subprojects/acpica/source/components/utilities/utstate.c',
	'../../subprojects/acpica/source/components/utilities/utosi.c',
	'../../subprojects/acpica/source/components/utilities/utresrc.c',
	'../../subprojects/acpica/source/components/utilities/utxferror.c',
	'../../subprojects/acpica/source/components/utilities/utxface.c',
	'../../subprojects/acpica/source/components/utilities/utxfinit.c'
)

trampoline = custom_target('trampoline',
	command: c_compiler.cmd_array() + ['-o', '@OUTPUT@',
		'-nostdlib', '-Wl,-Ttext,0', '-Wl,--oformat,binary',
		'@INPUT@'],
	input: 'extra-src/trampoline.S',
	output: 'trampoline.bin')

embed_trampoline = custom_target('embed-trampoline',
	command: ['x86_64-managarm-objcopy', '-I', 'binary', '-O', 'elf64-x86-64',
			'-B', 'i386:x86-64', '@INPUT@', '@OUTPUT@'],
	input: trampoline,
	output: 'embed-trampoline.o')

executable('thor', pb_sources, thor_sources, acpica_sources, embed_trampoline,
	include_directories: include_directories(
		'c_headers/', '../include/',
		'../common',
		'../../frigg/include',
		'../../libarch/include',
		'../../subprojects/acpica/source/include'),
	dependencies: [cxxshim_dep, frigg_dep, libsmarter_dep],
	c_args: ['-std=c11', '-ffreestanding',
		'-mcmodel=kernel', '-mno-red-zone',
		'-msoft-float', '-mno-sse', '-mno-mmx', '-mno-sse2', '-mno-3dnow', '-mno-avx',
		'-Wall'],
	cpp_args: ['-std=c++17', '-ffreestanding', '-fno-exceptions', '-fno-rtti',
		'-mcmodel=kernel', '-mno-red-zone',
		'-msoft-float', '-mno-sse', '-mno-mmx', '-mno-sse2', '-mno-3dnow', '-mno-avx',
		'-DCXXSHIM_INTEGRATE_GCC', '-DFRIGG_NO_LIBC',
		'-Wall', '-Wno-non-virtual-dtor'],
	link_args: ['-nostdlib', '-z', 'max-page-size=0x1000',
		'-T', meson.current_source_dir() + '/arch/x86/link.x'],
	link_depends: files('arch/x86/link.x'))
