/*  FILE: agt_util.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
24may06      abb      begun; split out from agt_ncx.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>
#include  <string.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_cap
#include "agt_cap.h"
#endif

#ifndef _H_agt_rpc
#include "agt_rpc.h"
#endif

#ifndef _H_agt_rpcerr
#include "agt_rpcerr.h"
#endif

#ifndef _H_agt_tree
#include "agt_tree.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val
#include "agt_val.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_obj
#include  "obj.h"
#endif

#ifndef _H_op
#include  "op.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_msg
#include  "xml_msg.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

#ifndef _H_xml_wr
#include  "xml_wr.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define AGT_UTIL_DEBUG  1


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION get_dataclass
*
* Get the dataclass field from the node
*
* INPUTS:
*    nodetyp == ncx_node_t enum for node
*    node == node to get dataclass from
*
* RETURNS:
*    data class
*********************************************************************/
static ncx_data_class_t
    get_dataclass (ncx_node_t nodetyp,
		   const void *node)
{
    const val_value_t *val;

    switch (nodetyp) {
    case NCX_NT_VAL:
	val = node;
	return val->dataclass;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NCX_DC_NONE;
    }
    /*NOTREACHED*/

} /* get_dataclass */


/********************************************************************
* FUNCTION is_default
*
* Check if the node is set to the default value
*
* INPUTS:
*    nodetyp == ncx_node_t enum for node
*    node == node to get dataclass from
*
* RETURNS:
*    TRUE if set to the default value (by user or agent)
*    FALSE if no default applicable or not set to default
*********************************************************************/
static boolean
    is_default (ncx_node_t nodetyp,
		const void *node)
{
    const val_value_t *val;

    switch (nodetyp) {
    case NCX_NT_VAL:
	val = node;
	return val_is_default(val);
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }
    /*NOTREACHED*/

} /* is_default */


/************  E X T E R N A L    F U N C T I O N S    **************/


/********************************************************************
* FUNCTION agt_get_cfg_from_parm
*
* Get the cfg_template_t for the config in the target param
*
* INPUTS:
*    parmname == parameter to get from (e.g., target)
*    msg == incoming rpc_msg_t in progress
*    methnode == XML node for RPC method (for errors)
*    retcfg == address of return cfg pointer
* 
* OUTPUTS:
*   *retcfg is set to the address of the cfg_template_t struct 
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_get_cfg_from_parm (const xmlChar *parmname,
			   rpc_msg_t *msg,
			   xml_node_t *methnode,
			   cfg_template_t  **retcfg)
{
    cfg_template_t    *cfg;
    val_value_t       *val;
    const val_value_t *errval;
    const xmlChar     *cfgname;
    status_t           res;

    val = val_find_child(&msg->rpc_input, 
			 obj_get_mod_name(msg->rpc_input.obj), 
			 parmname);
    if (!val || val->res != NO_ERR) {
	if (!val) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else {
	    res = val->res;
	}
	agt_record_error(NULL, &msg->mhdr.errQ, NCX_LAYER_OPERATION,
			 res, methnode, NCX_NT_NONE, NULL, NCX_NT_VAL, 
			 &msg->rpc_input);
	return res;
    }

    errval = val;
    cfgname = NULL;
    res = NO_ERR;

    /* got some value in *val */
    switch (val->btyp) {
    case NCX_BT_STRING:
	cfgname = VAL_STR(val);
	break;
    case NCX_BT_EMPTY:
	cfgname = val->name;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (cfgname) {
	/* get the config template from the config name */
	cfg = cfg_get_config(cfgname);
	if (!cfg) {
	    res = ERR_NCX_CFG_NOT_FOUND;
	}
    }

    if (res != NO_ERR) {
	agt_record_error(NULL, &msg->mhdr.errQ, NCX_LAYER_OPERATION,
			 res, methnode,
			 (cfgname) ? NCX_NT_STRING : NCX_NT_NONE,
			 (const void *)cfgname, 
			 NCX_NT_VAL, errval);
	return res;
    }

    *retcfg = cfg;
    return NO_ERR;

} /* agt_get_cfg_from_parm */


/********************************************************************
* FUNCTION agt_get_parmval
*
* Get the identified val_value_t for a given parameter
* Used for error purposes!
* INPUTS:
*    parmname == parameter to get
*    msg == incoming rpc_msg_t in progress
*
* RETURNS:
*    status
*********************************************************************/
const val_value_t *
    agt_get_parmval (const xmlChar *parmname,
		     rpc_msg_t *msg)
{
    val_value_t *val;

    val =  val_find_child(&msg->rpc_input,
			  obj_get_mod_name(msg->rpc_input.obj),
			  parmname);
    return val;

} /* agt_get_parmval */


/********************************************************************
* FUNCTION agt_record_error
*
* Generate an rpc_err_rec_t and save it in the msg
*
* INPUTS:
*    scb == session control block 
*        == NULL and no stats will be recorded
*    errQ == errQ to record error 
*         == NULL, no error will be recorded!
*    layer == netconf layer error occured               <error-type>
*    res == internal error code                      <error-app-tag>
*    xmlnode == XML node causing error  <bad-element>   <error-path> 
*            == NULL if not available 
*    parmtyp == type of node in 'error_parm'
*    error_parm == error data, specific to 'res'        <error-info>
*               == NULL if not available (then nodetyp ignred)
*    nodetyp == type of node in 'errnode'
*    errnode == internal data node with the error       <error-path>
*            == NULL if not available or not used  
* OUTPUTS:
*   errQ has error message added if no malloc errors
*   scb->stats may be updated if scb non-NULL

* RETURNS:
*    none
*********************************************************************/
void
    agt_record_error (ses_cb_t *scb,
		      dlq_hdr_t *errQ,
		      ncx_layer_t layer,
		      status_t  res,
		      const xml_node_t *xmlnode,
		      ncx_node_t parmtyp,
		      const void *error_parm,
		      ncx_node_t nodetyp,
		      const void *errnode)
{
    rpc_err_rec_t   *err;
    xmlChar         *buff;

    if (LOGDEBUG3) {
	log_debug3("\nagt_record_error: ");
	if (xmlnode) {
	    log_debug3(" xml: %u:%s", 
		       xmlns_get_ns_prefix(xmlnode->nsid),
		       xmlnode->elname ? xmlnode->elname : (const xmlChar *)"--");
	}
	if (nodetyp == NCX_NT_VAL && errnode) {
	    log_debug3(" errnode: \n");
	    val_dump_value((const val_value_t *)errnode, NCX_DEF_INDENT);
	    log_debug3("\n");
	}
    }

    if (errQ) {
	buff = NULL;
	if (errnode) {
	    switch (nodetyp) {
	    case NCX_NT_STRING:
		buff = xml_strdup((const xmlChar *)errnode);
		break;
	    case NCX_NT_VAL:
		(void)val_gen_instance_id(errnode, 
					  NCX_IFMT_XPATH1, 
					  TRUE, &buff);
		break;
	    case NCX_NT_OBJ:
		(void)obj_gen_object_id(errnode, &buff);
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}

	err = agt_rpcerr_gen_error(layer, res, xmlnode, 
				   parmtyp, error_parm, buff);
	if (err) {
	    dlq_enque(err, errQ);
	} else {
	    if (buff) {
		m__free(buff);
	    }
	    /*** inc error-dropped counter for the session stats ***/
	    if (scb) {
		;
	    }
	}
    }

    /* TBD: inc the specific error type for the session stats */
    if (scb) {
	scb->stats.in_err_msgs++;
    }

} /* agt_record_error */


/********************************************************************
* FUNCTION agt_record_attr_error
*
* Generate an rpc_err_rec_t and save it in the msg
*
* INPUTS:
*    scb == session control block
*    errQ == error Q to record error
*    layer == netconf layer error occured
*    res == internal error code
*    xmlnode == XML node causing error (NULL if not available)
*    error_parm == error data, specific to 'res'
*    nodetyp == type of node in 'errnode'
*    errnode == internal data node with the error
*            == NULL if not used
* OUTPUTS:
*   errQ has error message added if no malloc errors
* RETURNS:
*    none
*********************************************************************/
void
    agt_record_attr_error (ses_cb_t *scb,
			   dlq_hdr_t *errQ,
			   ncx_layer_t layer,
			   status_t  res,
			   const xml_attr_t *xmlattr,
			   const xml_node_t *xmlnode,
			   const xmlChar *badns,
			   ncx_node_t nodetyp,
			   const void *errnode)
{
    rpc_err_rec_t   *err;
    xmlChar         *buff;

    if (errQ) {
	buff = NULL;
	if (errnode) {
	    if (nodetyp==NCX_NT_STRING) {
		buff = xml_strdup((const xmlChar *)errnode);
	    } else {
		(void)val_gen_instance_id(errnode, 
					  NCX_IFMT_XPATH2, 
					  TRUE, &buff);
	    }
	}
	err = agt_rpcerr_gen_attr_error(layer, res, xmlattr, 
				    xmlnode, badns, buff);
	if (err) {
	    dlq_enque(err, errQ);
	} else {
	    if (buff) {
		m__free(buff);
	    }
	    /*** inc error-dropped counter for the session stats ***/
	    if (scb) {
		;
	    }
	}
    }

    /* inc the specific error type for the session stats */
    if (scb) {
	scb->stats.in_err_msgs++;
    }

} /* agt_record_attr_error */


/********************************************************************
* FUNCTION agt_validate_filter
*
* Validate the <filter> parameter if present
*
* INPUTS:
*    scb == session control block
*    msg == rpc_msg_t in progress
*
* OUTPUTS:
*    msg->rpc_filter is filled in if NO_ERR; type could be NONE
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_validate_filter (ses_cb_t *scb,
			 rpc_msg_t *msg)
{
    val_value_t    *filter, *filtertype, *sel;
    const xmlChar  *errstr;
    op_filtertyp_t  filtyp;
    status_t        res;
    xmlns_id_t      nsid;
    xml_attr_t      selattr;

#ifdef DEBUG
    if (!scb || !msg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    filter = NULL;
    filtertype = NULL;
    sel = NULL;
    errstr = NULL;
    nsid = xmlns_nc_id();
    res = NO_ERR;

    /* filter parm is optional */
    filter = val_find_child(&msg->rpc_input, 
			    NC_MODULE, NCX_EL_FILTER);
    if (!filter) {
	msg->rpc_filter.op_filtyp = OP_FILTER_NONE;
	msg->rpc_filter.op_filter = NULL;
	return NO_ERR;   /* not an error */
    } else if (filter->res == NO_ERR) {
	/* setup the filter parameters */
	filtertype = val_find_meta(filter, nsid, NCX_EL_TYPE);
	if (!filtertype) {
	    /* should not happen; the default is subtree */
	    filtyp = OP_FILTER_SUBTREE;
	} else {
	    filtyp = op_filtertyp_id(VAL_STR(filtertype));
	}

	/* check if the select attribute is needed */
	switch (filtyp) {
	case OP_FILTER_SUBTREE:
	    break;
	case OP_FILTER_XPATH:
	    if (!agt_cap_std_set(CAP_STDID_XPATH)) {
		res = ERR_NCX_OPERATION_NOT_SUPPORTED;
		errstr = (const xmlChar *)"xpath";
	    } else {
		sel = val_find_meta(filter, nsid, NCX_EL_SELECT);
		if (!sel) {
		    memset(&selattr, 0x0, sizeof(xml_attr_t));
		    selattr.attr_ns = xmlns_nc_id();
		    selattr.attr_name = NCX_EL_SELECT;
		    res = ERR_NCX_MISSING_ATTRIBUTE;
		    agt_record_attr_error(scb, &msg->mhdr.errQ, 
					  NCX_LAYER_OPERATION, res,
					  &selattr, NULL, NULL,
					  NCX_NT_VAL, filter);
		    return res;
		}
	    }
	    break;
	case OP_FILTER_C:
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    errstr = (const xmlChar *)"C";
	    break;
	default:
	    res = ERR_NCX_INVALID_VALUE;
	}

	if (res != NO_ERR) {
	    agt_record_error(scb, &msg->mhdr.errQ, 
			     NCX_LAYER_OPERATION, res, NULL,
			     (errstr) ? NCX_NT_STRING : NCX_NT_NONE,
			     errstr, NCX_NT_VAL, filter);
	}
    } /* else optional filter parameter is not present */


    if (res == NO_ERR) {
#ifdef AGT_UTIL_DEBUG
	if (LOGDEBUG3) {
	    log_debug3("\nagt_util_validate_filter:");
	    val_dump_value(&msg->rpc_input, 0);
	}
#endif

	msg->rpc_filter.op_filtyp = filtyp;
	msg->rpc_filter.op_filter = (sel) ? sel : filter;
    }

    return res;

} /* agt_validate_filter */


/********************************************************************
* FUNCTION agt_check_config
*
* ncx_nodetest_fn_t callback
*
* Used by the <get-config> operation to return any type of 
* configuration data
*
* INPUTS:
*    see ncx/ncxtypes.h   (ncx_nodetest_fn_t)
* RETURNS:
*    status
*********************************************************************/
boolean
    agt_check_config (boolean withdef,
		      ncx_node_t nodetyp,
		      const void *node)
{
    ncx_data_class_t  dataclass;
    boolean ret;

    dataclass = get_dataclass(nodetyp, node);
    if (dataclass==NCX_DC_CONFIG) {
	ret = TRUE;

	/* check if defaults are suppressed */
	if (!withdef) {
	    /* with-defaults=false, check if this is a val with 
	     * a default value set
	     */
	    if (is_default(nodetyp, node)) {
		ret = FALSE;
	    }
	}
    } else {
	/* not a node that should be saved with a copy-config 
	 * to NVRAM
	 */
	ret = FALSE;
    }

    return ret;

} /* agt_check_config */


/********************************************************************
* FUNCTION agt_check_save
*
* ncx_nodetest_fn_t callback
*
* Used by agt_ncx_cfg_save function to filter just what
* is supposed to be saved in the <startup> config file
*
* INPUTS:
*    see ncx/ncxconst.h   (ncx_nodetest_fn_t)
* RETURNS:
*    status
*********************************************************************/
boolean
    agt_check_save (boolean withdef,
		    ncx_node_t nodetyp,
		    const void *node)
{
    ncx_data_class_t  dataclass;
    boolean ret;

    dataclass = get_dataclass(nodetyp, node);
    if (dataclass==NCX_DC_CONFIG) {
	ret = TRUE;

	/* check if defaults are suppressed */
	if (!withdef) {
	    /* with-defaults=false, check if this is a val with 
	     * a default value set
	     */
	    if (is_default(nodetyp, node)) {
		ret = FALSE;
	    }
	}
    } else {
	/* not a node that should be saved with a copy-config 
	 * to NVRAM
	 */
	ret = FALSE;
    }

    return ret;

} /* agt_check_save */


/********************************************************************
* FUNCTION agt_output_filter
*
* output the proper data for the get or get-config operation
*
* INPUTS:
*    see rpc/agt_rpc.h   (agt_rpc_data_cb_t)
* RETURNS:
*    status
*********************************************************************/
status_t
    agt_output_filter (ses_cb_t *scb,
		       rpc_msg_t *msg,
		       int32 indent)
{
    cfg_template_t *source;
    ncx_filptr_t   *top;
    boolean         getop;

    getop = !xml_strcmp(msg->rpc_meth_name, NCX_EL_GET);

    if (getop) {
	source = cfg_get_config_id(NCX_CFGID_RUNNING);
    } else {
	source = (cfg_template_t *)msg->rpc_user1;
    }
    if (!source) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }

    switch (msg->rpc_filter.op_filtyp) {
    case OP_FILTER_NONE:
	if (source->root) {
	    if (getop) {
		xml_wr_val(scb, &msg->mhdr, source->root, indent);
	    } else {
		xml_wr_check_val(scb, &msg->mhdr, source->root, 
				 indent, agt_check_config);
	    }
	}
	break;
    case OP_FILTER_C:
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    case OP_FILTER_SUBTREE:
	if (source->root) {
	    top = agt_tree_prune_filter(scb, msg, source, getop);
	    if (top) {
		agt_tree_output_filter(scb, msg, top, indent);
		ncx_free_filptr(top);
	    }
	}
	break;
    case OP_FILTER_XPATH:
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    default:
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    return NO_ERR;
		
} /* agt_output_filter */


/********************************************************************
* FUNCTION agt_check_max_access
* 
* Check if the max-access for a parameter is exceeded
*
* INPUTS:
*   op == requested op
*   acc == max-access for the parameter
*   cur_exists == TRUE if the corresponding node in the target exists
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_check_max_access (op_editop_t  op,
			  ncx_access_t acc,
			  boolean cur_exists)
{
    status_t  res;
    
    res = NO_ERR;
    switch (op) {
    case OP_EDITOP_NONE:
	return NO_ERR;
    case OP_EDITOP_MERGE:
	switch (acc) {
	case NCX_ACCESS_NONE:
	case NCX_ACCESS_RO:
	    return ERR_NCX_NO_ACCESS_MAX;
	case NCX_ACCESS_RW:
	    /* edit but not create is allowed */
	    return (cur_exists) ? NO_ERR : ERR_NCX_NO_ACCESS_MAX;
	case NCX_ACCESS_RC:
	    return NO_ERR;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    case OP_EDITOP_REPLACE:
	switch (acc) {
	case NCX_ACCESS_NONE:
	case NCX_ACCESS_RO:
	case NCX_ACCESS_RW:
	    return (cur_exists) ? NO_ERR : ERR_NCX_NO_ACCESS_MAX;
	case NCX_ACCESS_RC:
	    return NO_ERR;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    case OP_EDITOP_CREATE:
    case OP_EDITOP_DELETE:
	switch (acc) {
	case NCX_ACCESS_NONE:
	case NCX_ACCESS_RO:
	case NCX_ACCESS_RW:
	    return ERR_NCX_NO_ACCESS_MAX;
	case NCX_ACCESS_RC:
	    /* create/delete allowed */
	    return NO_ERR;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    case OP_EDITOP_LOAD:
	/* allow for agent loading of read-write objects */
	switch (acc) {
	case NCX_ACCESS_NONE:
	case NCX_ACCESS_RO:
	    return ERR_NCX_NO_ACCESS_MAX;
	case NCX_ACCESS_RW:
	case NCX_ACCESS_RC:
	    /* create/edit/delete allowed */
	    return NO_ERR;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);	
    }
    /*NOTREACHED*/

} /* agt_check_max_access */


/********************************************************************
* FUNCTION agt_check_editop
* 
* Check if the edit operation is okay
*
* INPUTS:
*   pop == parent edit operation in affect
*          Starting at the data root, the parent edit-op
*          is derived from the default-operation parameter
*   cop == address of child operation (MAY BE ADJUSTED ON EXIT!!!)
*          This is the edit-op field in the new node corresponding
*          to the curnode position in the data model
*   newnode == pointer to new node in the edit-config PDU
*   curnode == pointer to the current node in the data model
*              being affected by this operation, if any.
*           == NULL if no current node exists
*   iqual == effective instance qualifier for this value
* 
* OUTPUTS:
*   *cop may be adjusted to simplify further processing,
*    based on the following reduction algorithm:
*
*    create, replace, and delete operations are 'sticky'.
*    Once set, any nested edit-ops must be valid
*    within the context of the fixed operation (create or delete)
*    and the child operation gets changed to the sticky parent edit-op
*
*   if the parent is create or delete, and the child
*   is merge or replace, then the child can be ignored
*   and will be changed to the parent, since the create
*   or delete must be carried through all the way to the leaves
*
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_check_editop (op_editop_t   pop,
		      op_editop_t  *cop,
		      const val_value_t *newnode,
		      const val_value_t *curnode,
		      ncx_iqual_t iqual)
{
    status_t           res;

    res = NO_ERR;

    /* adjust the child if it has not been set 
     * this heuristic saves the real operation on the way down
     * the tree as each node is checked for a new operation value 
     *
     * This will either be the default-operation or a value
     * set in an anscestor node with an operation attribute
     *
     * If this is the internal startup mode (OP_EDITOP_LOAD)
     * then the load edit-op will get set and quick exit
     */
    if (*cop==OP_EDITOP_NONE) {
	*cop = pop;
    }

    /* check the child editop against the parent editop */
    switch (*cop) {
    case OP_EDITOP_NONE:
	/* no operation set in the child or the parent yet */
	res = (curnode) ? NO_ERR : ERR_NCX_DATA_MISSING;
	break;
    case OP_EDITOP_MERGE:
    case OP_EDITOP_REPLACE:
	switch (pop) {
	case OP_EDITOP_NONE:
	    /* this child contains the merge or replace operation 
	     * attribute; which may be an index node; although
	     * loose from a DB API POV, NETCONF will allow an
	     * entry to be renamed via a merge or replace edit-op
	     */
	    break;
	case OP_EDITOP_MERGE:
	case OP_EDITOP_REPLACE:
	    /* merge or replace inside merge or replace is okay */
	    break;
	case OP_EDITOP_CREATE:
	    /* a merge or replace within a create is okay
	     * but it is really a create because the parent
	     * operation is an explicit create, so the current
	     * node is not allowed to exist yet, unless multiple
	     * instances of the node are allowed
	     */
	    *cop = OP_EDITOP_CREATE;
	    if (curnode) {
		switch (iqual) {
		case NCX_IQUAL_ONE:
		case NCX_IQUAL_OPT:
		    res = ERR_NCX_DATA_EXISTS;
		    break;
		default:
		    ;
		}
	    }
	    break;
	case OP_EDITOP_DELETE:
	    /* this is an error since the merge or replace
	     * cannot be performed and its parent node is
	     * also getting deleted at the same time
	     */
	    res = ERR_NCX_DATA_MISSING;
	    break;
	case OP_EDITOP_LOAD:
	    /* LOAD op not allowed here */
	    res = ERR_NCX_BAD_ATTRIBUTE;
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
	break;
    case OP_EDITOP_CREATE:
	/* the child op is an explicit create
	 * the current node cannot exist unless multiple
	 * instances are allowed
	 */
	if (curnode) {
	    switch (iqual) {
	    case NCX_IQUAL_ONE:
	    case NCX_IQUAL_OPT:
		return ERR_NCX_DATA_EXISTS;
	    default:
		;
	    }
	}

	/* check the create op against the parent edit-op */
	switch (pop) {
	case OP_EDITOP_NONE:
	    /* make sure the create edit-op is in a correct place */
	    res = (val_create_allowed(newnode)) ?
		NO_ERR : ERR_NCX_OPERATION_FAILED;
	    break;
	case OP_EDITOP_MERGE:
	case OP_EDITOP_REPLACE:
	    /* create within merge or replace okay since these
	     * operations silently create any missing nodes
	     * and the curnode test already passed
	     */
	    break;
	case OP_EDITOP_CREATE:
	    /* create inside create okay */
	    break;
	case OP_EDITOP_DELETE:
	    /* create inside a delete is an error */
	    res = ERR_NCX_DATA_MISSING;
	    break;
	case OP_EDITOP_LOAD:
	    /* LOAD op not allowed here */
	    res = ERR_NCX_BAD_ATTRIBUTE;
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
	break;
    case OP_EDITOP_DELETE:
	/* explicit delete means the current node must exist
	 * unlike a replace which removes nodes if they exist,
	 * without any error checking for curnode exists
	 */
	if (!curnode) {
	    /* delete on non-existing node is always an error */
	    res = ERR_NCX_DATA_MISSING;
	} else {
	    /* check the delete against the parent edit-op */
	    switch (pop) {
	    case OP_EDITOP_NONE:
		/* make sure the create edit-op is in a correct place */
		res = (val_delete_allowed(curnode)) ?
		    NO_ERR : ERR_NCX_OPERATION_FAILED;
		break;
	    case OP_EDITOP_MERGE:
		/* delete within merge or ok */
		break;
	    case OP_EDITOP_REPLACE:
		/* this is a corner case; delete within a replace
		 * the application could have just left this node
		 * out instead, but allow this form too
		 */
		break;
	    case OP_EDITOP_CREATE:
		/* create within a delete always an error */
		res = ERR_NCX_DATA_MISSING;
		break;
	    case OP_EDITOP_DELETE:
		/* delete within delete always okay */
		break;
	    case OP_EDITOP_LOAD:
		/* LOAD op not allowed here */
		res = ERR_NCX_BAD_ATTRIBUTE;
		break;
	    default:
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
	break;
    case OP_EDITOP_LOAD:
	if (pop != OP_EDITOP_LOAD) {
	    res = ERR_NCX_BAD_ATTRIBUTE;
	}
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    return res;

} /* agt_check_editop */


/* END file agt_util.c */
