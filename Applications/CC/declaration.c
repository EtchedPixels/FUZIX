/*
 *	Declaration handling. All cases are actually the same, including
 *	typedef (hence the weird typedef syntax)
 */

#include "compiler.h"


unsigned one_typedef(unsigned type, unsigned name)
{
	if (name == 0) {
		error("invalid typedef");
		junk();
		return 0;
	}
	update_symbol_by_name(name, S_TYPEDEF, type);
	return 1;
}

void dotypedef(void)
{
	unsigned type = get_type();
	unsigned name;

	if (type == UNKNOWN)
		type = CINT;

//	while (is_modifier() || is_type_word() || token >= T_SYMBOL || token == T_STAR) {
	while (token != T_SEMICOLON) {
		unsigned utype = type_name_parse(S_NONE, type, &name);
		if (one_typedef(utype, name) == 0)
			return;
		if (!match(T_COMMA))
			break;
	}
	need_semicolon();
}

unsigned one_declaration(unsigned s, unsigned type, unsigned name, unsigned defstorage)
{
	struct symbol *sym;
	unsigned offset;

	/* It's quite valid C to just write "int;" but usually dumb except
	   that it's used for struct and union */
	if (name == 0) {
		if (!IS_STRUCT(type))
			warning("useless declaration");
		return 1;
	}
	if ((s == S_AUTO || s == S_REGISTER) && defstorage == S_EXTDEF)
		error("no automatic globals");

	if (IS_FUNCTION(type) && !PTR(type) && s == S_EXTDEF)
		s = S_EXTERN;

	if (s == S_REGISTER) {
		offset = target_register(type);
		if (offset == 0)
			s = S_AUTO;
	}

	/* Do we already have this symbol */
	sym = update_symbol_by_name(name, s, type);

	if (funcbody)
		return 0;

	if (s == S_REGISTER)
		sym->data.offset = offset;
	if (s == S_AUTO)
		sym->data.offset = assign_storage(type, S_AUTO);
	if (s == S_LSTATIC)
		sym->data.offset = ++label_tag;;

	if (s != S_EXTERN && (PTR(type) || !IS_FUNCTION(type)) && match(T_EQ)) {
		unsigned label = sym->name;
		if (s == S_LSTATIC)
			label = sym->data.offset;
		if (sym->infonext & INITIALIZED)
			error("duplicate initializer");
		sym->infonext |= INITIALIZED;
		if (s >= S_LSTATIC)
		        header(H_DATA, label, target_alignof(type, s));
		initializers(sym, type, s);
		if (s >= S_LSTATIC)
		        footer(H_DATA, label, 0);
	}
	return 1;
}

void declaration(unsigned defstorage)
{
	unsigned s = get_storage(defstorage);
	unsigned name;
	unsigned utype;
	unsigned type;

	type = get_type();
	if (type == UNKNOWN)
		type = CINT;

//	while (is_modifier() || is_type_word() || token >= T_SYMBOL || token == T_STAR) {
	while (token != T_SEMICOLON) {
		utype = type_name_parse(s, type, &name);
		if (one_declaration(s, utype, name, defstorage) == 0)
			return;
		if (!match(T_COMMA))
			break;
	}
	need_semicolon();
}
