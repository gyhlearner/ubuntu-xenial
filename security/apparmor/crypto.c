/*
 * AppArmor security module
 *
 * This file contains AppArmor policy loading interface function definitions.
 *
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 * Fns to provide a checksum of policy that has been loaded this can be
 * compared to userspace policy compiles to check loaded policy is what
 * it should be.
 */

#include <crypto/hash.h>

#include "include/apparmor.h"
#include "include/crypto.h"

static unsigned int apparmor_hash_size;

static struct crypto_shash *apparmor_tfm;

unsigned int aa_hash_size(void)
{
	return apparmor_hash_size;
}

<<<<<<< HEAD
char *aa_calc_hash(void *data, size_t len)
{
	struct {
		struct shash_desc shash;
		char ctx[crypto_shash_descsize(apparmor_tfm)];
	} desc;
=======
void aa_snprint_hashstr(char *out, unsigned char *hash, unsigned int hsize)
{
       unsigned int i;

       for (i = 0; i < hsize; i++)
               sprintf(out + i*2, "%.2x", hash[i]);
       out[hsize*2] = 0;
}

char *aa_asprint_hashstr(unsigned char *hash, unsigned int hsize, gfp_t gfp)
{
	char *buffer = kmalloc(hsize*2 + 1, gfp);
	if (!buffer)
		return NULL;
	aa_snprint_hashstr(buffer, hash, hsize);

	return buffer;
}

char *aa_calc_hash(void *data, size_t len)
{
	SHASH_DESC_ON_STACK(desc, apparmor_tfm);
>>>>>>> temp
	char *hash = NULL;
	int error = -ENOMEM;

	if (!apparmor_tfm)
		return NULL;

	hash = kzalloc(apparmor_hash_size, GFP_KERNEL);
	if (!hash)
		goto fail;

<<<<<<< HEAD
	desc.shash.tfm = apparmor_tfm;
	desc.shash.flags = 0;

	error = crypto_shash_init(&desc.shash);
	if (error)
		goto fail;
	error = crypto_shash_update(&desc.shash, (u8 *) data, len);
	if (error)
		goto fail;
	error = crypto_shash_final(&desc.shash, hash);
=======
	desc->tfm = apparmor_tfm;
	desc->flags = 0;

	error = crypto_shash_init(desc);
	if (error)
		goto fail;
	error = crypto_shash_update(desc, (u8 *) data, len);
	if (error)
		goto fail;
	error = crypto_shash_final(desc, hash);
>>>>>>> temp
	if (error)
		goto fail;

	return hash;

fail:
	kfree(hash);

	return ERR_PTR(error);
}

int aa_calc_profile_hash(struct aa_profile *profile, u32 version, void *start,
			 size_t len)
{
	SHASH_DESC_ON_STACK(desc, apparmor_tfm);
	int error = -ENOMEM;
	__le32 le32_version = cpu_to_le32(version);

	if (!aa_g_hash_policy)
		return 0;

	if (!aa_g_hash_policy)
		return 0;

	if (!apparmor_tfm)
		return 0;

	profile->hash = kzalloc(apparmor_hash_size, GFP_KERNEL);
	if (!profile->hash)
		goto fail;

	desc->tfm = apparmor_tfm;
	desc->flags = 0;

	error = crypto_shash_init(desc);
	if (error)
		goto fail;
	error = crypto_shash_update(desc, (u8 *) &le32_version, 4);
	if (error)
		goto fail;
	error = crypto_shash_update(desc, (u8 *) start, len);
	if (error)
		goto fail;
	error = crypto_shash_final(desc, profile->hash);
	if (error)
		goto fail;

	return 0;

fail:
	kfree(profile->hash);
	profile->hash = NULL;

	return error;
}

static int __init init_profile_hash(void)
{
	struct crypto_shash *tfm;

	if (!apparmor_initialized)
		return 0;

	tfm = crypto_alloc_shash("sha1", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		int error = PTR_ERR(tfm);
		AA_ERROR("failed to setup profile sha1 hashing: %d\n", error);
		return error;
	}
	apparmor_tfm = tfm;
	apparmor_hash_size = crypto_shash_digestsize(apparmor_tfm);

	aa_info_message("AppArmor sha1 policy hashing enabled");

	return 0;
}

late_initcall(init_profile_hash);
