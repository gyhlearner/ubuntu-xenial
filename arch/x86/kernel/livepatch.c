/*
 * livepatch.c - x86-specific Kernel Live Patching Core
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
<<<<<<< HEAD
#include <linux/uaccess.h>
#include <asm/elf.h>
#include <asm/livepatch.h>
=======
#include <linux/kallsyms.h>
#include <linux/livepatch.h>
#include <asm/text-patching.h>
>>>>>>> temp

/* Apply per-object alternatives. Based on x86 module_finalize() */
void arch_klp_init_object_loaded(struct klp_patch *patch,
				 struct klp_object *obj)
{
<<<<<<< HEAD
	size_t size = 4;
	unsigned long val;
	unsigned long core = (unsigned long)mod->core_layout.base;
	unsigned long core_size = mod->core_layout.size;

	switch (type) {
	case R_X86_64_NONE:
		return 0;
	case R_X86_64_64:
		val = value;
		size = 8;
		break;
	case R_X86_64_32:
		val = (u32)value;
		break;
	case R_X86_64_32S:
		val = (s32)value;
		break;
	case R_X86_64_PC32:
		val = (u32)(value - loc);
		break;
	default:
		/* unsupported relocation type */
		return -EINVAL;
	}

	if (loc < core || loc >= core + core_size)
		/* loc does not point to any symbol inside the module */
		return -EINVAL;

	return probe_kernel_write((void *)loc, &val, size);
=======
	int cnt;
	struct klp_modinfo *info;
	Elf_Shdr *s, *alt = NULL, *para = NULL;
	void *aseg, *pseg;
	const char *objname;
	char sec_objname[MODULE_NAME_LEN];
	char secname[KSYM_NAME_LEN];

	info = patch->mod->klp_info;
	objname = obj->name ? obj->name : "vmlinux";

	/* See livepatch core code for BUILD_BUG_ON() explanation */
	BUILD_BUG_ON(MODULE_NAME_LEN < 56 || KSYM_NAME_LEN != 128);

	for (s = info->sechdrs; s < info->sechdrs + info->hdr.e_shnum; s++) {
		/* Apply per-object .klp.arch sections */
		cnt = sscanf(info->secstrings + s->sh_name,
			     ".klp.arch.%55[^.].%127s",
			     sec_objname, secname);
		if (cnt != 2)
			continue;
		if (strcmp(sec_objname, objname))
			continue;
		if (!strcmp(".altinstructions", secname))
			alt = s;
		if (!strcmp(".parainstructions", secname))
			para = s;
	}

	if (alt) {
		aseg = (void *) alt->sh_addr;
		apply_alternatives(aseg, aseg + alt->sh_size);
	}

	if (para) {
		pseg = (void *) para->sh_addr;
		apply_paravirt(pseg, pseg + para->sh_size);
	}
>>>>>>> temp
}
