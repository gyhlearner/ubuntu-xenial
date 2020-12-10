/* Asymmetric public-key algorithm definitions
 *
 * See Documentation/crypto/asymmetric-keys.txt
 *
 * Copyright (C) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version
 * 2 of the Licence, or (at your option) any later version.
 */

#ifndef _LINUX_PUBLIC_KEY_H
#define _LINUX_PUBLIC_KEY_H

<<<<<<< HEAD
#include <linux/mpi.h>
#include <crypto/hash_info.h>

enum pkey_algo {
	PKEY_ALGO_DSA,
	PKEY_ALGO_RSA,
	PKEY_ALGO__LAST
};

extern const char *const pkey_algo_name[PKEY_ALGO__LAST];

/* asymmetric key implementation supports only up to SHA224 */
#define PKEY_HASH__LAST		(HASH_ALGO_SHA224 + 1)

enum pkey_id_type {
	PKEY_ID_PGP,		/* OpenPGP generated key ID */
	PKEY_ID_X509,		/* X.509 arbitrary subjectKeyIdentifier */
	PKEY_ID_PKCS7,		/* Signature in PKCS#7 message */
	PKEY_ID_TYPE__LAST
};

extern const char *const pkey_id_type_name[PKEY_ID_TYPE__LAST];

/*
 * The use to which an asymmetric key is being put.
 */
enum key_being_used_for {
	VERIFYING_MODULE_SIGNATURE,
	VERIFYING_FIRMWARE_SIGNATURE,
	VERIFYING_KEXEC_PE_SIGNATURE,
	VERIFYING_KEY_SIGNATURE,
	VERIFYING_KEY_SELF_SIGNATURE,
	VERIFYING_UNSPECIFIED_SIGNATURE,
	NR__KEY_BEING_USED_FOR
};
extern const char *const key_being_used_for[NR__KEY_BEING_USED_FOR];

=======
>>>>>>> temp
/*
 * Cryptographic data for the public-key subtype of the asymmetric key type.
 *
 * Note that this may include private part of the key as well as the public
 * part.
 */
struct public_key {
	void *key;
	u32 keylen;
<<<<<<< HEAD
	enum pkey_algo pkey_algo : 8;
	enum pkey_id_type id_type : 8;
=======
	const char *id_type;
	const char *pkey_algo;
>>>>>>> temp
};

extern void public_key_free(struct public_key *key);

/*
 * Public key cryptography signature data
 */
struct public_key_signature {
<<<<<<< HEAD
=======
	struct asymmetric_key_id *auth_ids[2];
>>>>>>> temp
	u8 *s;			/* Signature */
	u32 s_size;		/* Number of bytes in signature */
	u8 *digest;
	u8 digest_size;		/* Number of bytes in digest */
	const char *pkey_algo;
	const char *hash_algo;
};

<<<<<<< HEAD
extern struct asymmetric_key_subtype public_key_subtype;
=======
extern void public_key_signature_free(struct public_key_signature *sig);

extern struct asymmetric_key_subtype public_key_subtype;

>>>>>>> temp
struct key;
struct key_type;
union key_payload;

extern int restrict_link_by_signature(struct key *dest_keyring,
				      const struct key_type *type,
				      const union key_payload *payload,
				      struct key *trust_keyring);

extern int restrict_link_by_key_or_keyring(struct key *dest_keyring,
					   const struct key_type *type,
					   const union key_payload *payload,
					   struct key *trusted);

extern int restrict_link_by_key_or_keyring_chain(struct key *trust_keyring,
						 const struct key_type *type,
						 const union key_payload *payload,
						 struct key *trusted);

extern int verify_signature(const struct key *key,
			    const struct public_key_signature *sig);

int public_key_verify_signature(const struct public_key *pkey,
				const struct public_key_signature *sig);

int public_key_verify_signature(const struct public_key *pkey,
				const struct public_key_signature *sig);

int rsa_verify_signature(const struct public_key *pkey,
			 const struct public_key_signature *sig);
#endif /* _LINUX_PUBLIC_KEY_H */
