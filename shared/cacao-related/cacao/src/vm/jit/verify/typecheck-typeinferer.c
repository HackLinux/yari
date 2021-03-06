/* src/vm/jit/verify/typecheck-typeinferer.c - type inference pass

   Copyright (C) 1996-2005, 2006, 2007 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   J. Wenninger, Institut f. Computersprachen - TU Wien

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   $Id$

*/

#include "config.h"
#include "vm/types.h"
#include "vm/global.h"

#include <assert.h>
#include <string.h>

#include "mm/memory.h"
#include "toolbox/logging.h"
#include "native/native.h"

#include "vm/access.h"
#include "vm/builtin.h"
#include "vm/exceptions.h"
#include "vm/vm.h"

#include "vm/jit/patcher.h"
#include "vm/jit/jit.h"
#include "vm/jit/show.h"
#include "vm/jit/parse.h"

#include "vm/jit/verify/typecheck-typeinferer.h"

#include "vmcore/loader.h"
#include "vmcore/options.h"
#include "vm/resolve.h"

#define TYPECHECK_NO_STATISTICS
#include <typecheck-common.h>


/* macros used by the generated code ******************************************/

#define EXCEPTION          do { return false; } while (0)
#define VERIFY_ERROR(msg)  assert(false)

#define CHECK_LOCAL_TYPE(index, t)                                   \
	assert(jd->var[(index)].type == (t));

#define STORE_LOCAL(t, index)                                        \
    do {                                                             \
         typevector_store(jd->var, (index), (t), NULL);              \
    } while (0)

#define STORE_LOCAL_2_WORD(t, index)                                 \
    do {                                                             \
         typevector_store(jd->var, (index), (t), NULL);              \
    } while (0)

#define REACH_BLOCK(target)                                          \
    do {                                                             \
        if (!typestate_reach(state, (target),                        \
                             state->bptr->outvars, jd->var,          \
                             state->bptr->outdepth))                 \
                return false;                                        \
    } while (0)

#define REACH(target)   REACH_BLOCK((target).block)

#define TYPECHECK_INT(v)  assert(jd->var[(v)].type == TYPE_INT)
#define TYPECHECK_ADR(v)  assert(jd->var[(v)].type == TYPE_ADR)


/* handle_fieldaccess **********************************************************
 
   Verify an ICMD_{GET,PUT}{STATIC,FIELD}(CONST)?
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_fieldaccess(verifier_state *state, 
				   varinfo *instance,
				   varinfo *value)
{
	jitdata *jd;

	jd = state->jd;

#define TYPECHECK_TYPEINFERER
#include <typecheck-fields.inc>
#undef  TYPECHECK_TYPEINFERER

	return true;
}


/* handle_invocation ***********************************************************
 
   Verify an ICMD_INVOKE* instruction.
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_invocation(verifier_state *state)
{
	jitdata *jd;
    varinfo *dv;               /* output variable of current instruction */

	jd = state->jd;
	dv = VAROP(state->iptr->dst);

#define TYPECHECK_TYPEINFERER
#define OP1   VAR(state->iptr->sx.s23.s2.args[0])
#include <typecheck-invoke.inc>
#undef  OP1
#undef  TYPECHECK_TYPEINFERER

	return true;
}


/* handle_builtin **************************************************************
 
   Verify the call of a builtin method.
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_builtin(verifier_state *state)
{
	jitdata *jd;
    varinfo *dv;               /* output variable of current instruction */

	jd = state->jd;
	dv = VAROP(state->iptr->dst);

#define TYPECHECK_TYPEINFERER
#define OP1   state->iptr->sx.s23.s2.args[0]
#include <typecheck-builtins.inc>
#undef  OP1
#undef  TYPECHECK_TYPEINFERER

	return true;
}

/* handle_multianewarray *******************************************************
 
   Verify a MULTIANEWARRAY instruction.
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_multianewarray(verifier_state *state)
{
	jitdata *jd;
    varinfo *dv;               /* output variable of current instruction */

	jd = state->jd;
	dv = VAROP(state->iptr->dst);

#define TYPECHECK_TYPEINFERER
#include <typecheck-multianewarray.inc>
#undef  TYPECHECK_TYPEINFERER

	return true;
}


/* handle_basic_block **********************************************************
 
   Perform bytecode verification of a basic block.
  
   IN:
       state............the current state of the verifier

   RETURN VALUE:
       true.............successful verification,
	   false............an exception has been thrown.

*******************************************************************************/

static bool
handle_basic_block(verifier_state *state)
{
    int opcode;                                      /* current opcode */
    int len;                        /* for counting instructions, etc. */
    bool superblockend;        /* true if no fallthrough to next block */
	instruction *iptr;                      /* the current instruction */
    basicblock *tbptr;                   /* temporary for target block */
    bool maythrow;               /* true if this instruction may throw */
	s4 i;
	branch_target_t *table;
	lookup_target_t *lookup;
	jitdata *jd = state->jd;
	exception_entry *ex;

	LOGSTR1("\n---- BLOCK %04d ------------------------------------------------\n",state->bptr->nr);
	LOGFLUSH;
	DOLOG(show_basicblock(jd, state->bptr, SHOW_STACK));

	superblockend = false;
	state->bptr->flags = BBFINISHED;

	/* prevent compiler warnings */


	/* determine the active exception handlers for this block */
	/* XXX could use a faster algorithm with sorted lists or  */
	/* something?                                             */
	len = 0;
	for (ex = state->jd->exceptiontable; ex ; ex = ex->down) {
		if ((ex->start->nr <= state->bptr->nr) && (ex->end->nr > state->bptr->nr)) {
			LOG1("active handler L%03d", ex->handler->nr);
			state->handlers[len++] = ex;
		}
	}
	state->handlers[len] = NULL;

	/* init variable types at the start of this block */
	typevector_copy_inplace(state->bptr->inlocals, jd->var, state->numlocals);

	DOLOG(typecheck_print_vararray(stdout, jd, state->bptr->invars, 
				state->bptr->indepth));
	DOLOG(typevector_print(stdout, jd->var, state->numlocals));
	LOGNL; LOGFLUSH;

	/* loop over the instructions */
	len = state->bptr->icount;
	state->iptr = state->bptr->iinstr;
	while (--len >= 0)  {
		TYPECHECK_COUNT(stat_ins);

		iptr = state->iptr;

		DOLOG(typevector_print(stdout, jd->var, state->numlocals));
		LOGNL; LOGFLUSH;
		DOLOG(show_icmd(jd, state->iptr, false, SHOW_STACK)); LOGNL; LOGFLUSH;

		opcode = iptr->opc;
		maythrow = false;

		switch (opcode) {

			/* include generated code for ICMDs verification */

#define TYPECHECK_TYPEINFERER
#define STATE  state
#define METHOD (state->m)
#define IPTR   iptr
#define BPTR   (state->bptr)
#include <typecheck-typeinferer-gen.inc>
#undef  STATE
#undef  METHOD
#undef  IPTR
#undef  BPTR
#undef  TYPECHECK_TYPEINFERER

			default:
				vm_abort("missing ICMD in type inferer: %d\n", opcode);
		}

		/* reach exception handlers for this instruction */

		if (maythrow) {
			TYPECHECK_COUNT(stat_ins_maythrow);
			TYPECHECK_MARK(state->stat_maythrow);
			LOG("reaching exception handlers");
			i = 0;
			while (state->handlers[i]) {
				TYPECHECK_COUNT(stat_handlers_reached);
				if (state->handlers[i]->catchtype.any)
					VAR(state->exinvars)->typeinfo.typeclass = state->handlers[i]->catchtype;
				else
					VAR(state->exinvars)->typeinfo.typeclass.cls = class_java_lang_Throwable;
				if (!typestate_reach(state,
						state->handlers[i]->handler,
						&(state->exinvars), jd->var, 1))
					return false;
				i++;
			}
		}

		LOG("\t\tnext instruction");
		state->iptr++;
	} /* while instructions */

	LOG("instructions done");
	LOGSTR("RESULT=> ");
	DOLOG(typecheck_print_vararray(stdout, jd, state->bptr->outvars,
				state->bptr->outdepth));
	DOLOG(typevector_print(stdout, jd->var, state->numlocals));
	LOGNL; LOGFLUSH;

	/* propagate stack and variables to the following block */
	if (!superblockend) {
		LOG("reaching following block");
		tbptr = state->bptr->next;
		while (tbptr->flags == BBDELETED) {
			tbptr = tbptr->next;
		}
		if (!typestate_reach(state,tbptr,state->bptr->outvars, jd->var,
					state->bptr->outdepth))
			return false;
	}

	return true;
}


bool typecheck_infer_types(jitdata *jd)
{
	methodinfo     *meth;
	codegendata    *cd;
	varinfo        *savedlocals;
	verifier_state  state;             /* current state of the verifier */

	/* get required compiler data */

	meth = jd->m;
	cd   = jd->cd;

	/* some logging on entry */


    LOGSTR("\n==============================================================================\n");
    DOLOG( show_method(jd, SHOW_STACK) );
    LOGSTR("\n==============================================================================\n");
    LOGMETHOD("Entering type inference: ",cd->method);

	/* initialize the verifier state */

	state.m = meth;
	state.jd = jd;
	state.cd = cd;
	state.basicblockcount = jd->basicblockcount;
	state.basicblocks = jd->basicblocks;
	state.savedindices = NULL;
	state.savedinvars = NULL;

	/* check that the basicblock numbers are valid */

#if !defined(NDEBUG)
	jit_check_basicblock_numbers(jd);
#endif

	/* check if this method is an instance initializer method */

    state.initmethod = (state.m->name == utf_init);

	/* initialize the basic block flags for the following CFG traversal */

	typecheck_init_flags(&state, BBFINISHED);

    /* number of local variables */
    
    /* In <init> methods we use an extra local variable to indicate whether */
    /* the 'this' reference has been initialized.                           */
	/*         TYPE_VOID...means 'this' has not been initialized,           */
	/*         TYPE_INT....means 'this' has been initialized.               */

    state.numlocals = state.jd->localcount;
	state.validlocals = state.numlocals;
    if (state.initmethod) 
		state.numlocals++; /* VERIFIER_EXTRA_LOCALS */

    /* allocate the buffer of active exception handlers */
	
    state.handlers = DMNEW(exception_entry*, state.jd->exceptiontablelength + 1);

	/* save local variables */

	savedlocals = DMNEW(varinfo, state.numlocals);
	MCOPY(savedlocals, jd->var, varinfo, state.numlocals);

	/* initialized local variables of first block */

	if (!typecheck_init_locals(&state, false))
		return false;

    /* initialize invars of exception handlers */
	
	state.exinvars = state.numlocals;
	VAR(state.exinvars)->type = TYPE_ADR;
	typeinfo_init_classinfo(&(VAR(state.exinvars)->typeinfo),
							class_java_lang_Throwable); /* changed later */

    LOG("Exception handler stacks set.\n");

    /* loop while there are still blocks to be checked */
    do {
		TYPECHECK_COUNT(count_iterations);

        state.repeat = false;
        
        state.bptr = state.basicblocks;

        for (; state.bptr; state.bptr = state.bptr->next) {
            LOGSTR1("---- BLOCK %04d, ",state.bptr->nr);
            LOGSTR1("blockflags: %d\n",state.bptr->flags);
            LOGFLUSH;
            
		    /* verify reached block */	
            if (state.bptr->flags == BBTYPECHECK_REACHED) {
                if (!handle_basic_block(&state))
					return false;
            }
        } /* for blocks */

        LOGIF(state.repeat,"state.repeat == true");
    } while (state.repeat);

	/* statistics */
	
	/* reset the flags of blocks we haven't reached */

	typecheck_reset_flags(&state);

	/* restore locals */

	MCOPY(jd->var, savedlocals, varinfo, state.numlocals);

	/* everything's ok */

    LOGimp("exiting type inference");
	return true;
}

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
