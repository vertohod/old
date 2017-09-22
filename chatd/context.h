#ifndef CONTEXT_H
#define CONTEXT_H

#include "sessions.h"
#include "response.h"
#include <string>
#include <memory>
#include "types.h"

class context
{
public:
	sptr<const UUID>		uuid_ptr;
	sptr<session>			sess_ptr;
	sptr_cstr				avatar;

	response				resp;

	OID		room;
	OID		last_mess;
	bool	typing;
	OID		to;
	OID		poid;
	bool	priv;
	bool	personal;
	OID		room_old;
	OID		room_new;
	int		answer_obj_mask;

	unsigned char	color;

	sptr_cstr		mess;
	sptr_cstr		nick;
	sptr_cstr		images;
	sptr_cstr		audios;
	sptr_cstr		videos;
	sptr_cstr		archives;
};

#endif
