/*
 * fault-injection-library.c
 *
 *  Created on: 07.08.2014
 *      Author: Gerhard Schoenfelder
 */

#include "fault-injection-library.h"
#include "fault-injection-data-analyzer.h"

//#include <unistd.h>
#include <libxml/xmlreader.h>
#include "profiler.h"
#include "qemu/error-report.h"
#include "qemu/timer.h"

/**
 * Linked list pointer to the first entry
 */
static FaultList *head = 0;
/**
 * Linked list pointer to the current entry
 */
static FaultList *curr = 0;
/**
 * num_list_elements contains the number of the
 * stored entries in the linked list.
 */
static int num_list_elements = 0;

/**
 * Array, which stores the previous
 * memory cell operations for
 * dynamic faults.
 */
static int **ops_on_memory_cell;

/**
 * Array, which stores the previous
 * register cell operations for
 * dynamic faults.
 */
static int **ops_on_register_cell;

char *fault_library_name;

/**
 * Allocates the size for the first entry in the linked list and parses the elements to it.
 *
 * @param[in] fault - all necessary elements for defining a fault, which are held
 *                              by the struct "Fault"
 * @param[out] ptr - pointer to the linked list entry (added element)
 */
static FaultList* create_fault_list(struct Fault *fault)
{
    FaultList *ptr = (FaultList*) malloc( sizeof(struct Fault) );
    if(ptr == 0)
    {
        error_printf("Node creation failed\n");
        return 0;
    }

    ptr->id = fault->id;
		ptr->component = fault->component;
		ptr->target = fault->target;
		ptr->mode = fault->mode;
		ptr->trigger = fault->trigger;
		ptr->timer = fault->timer;
		ptr->type = fault->type;
		ptr->duration = fault->duration;
		ptr->interval = fault->interval;
		ptr->params.address = fault->params.address;
		ptr->params.cf_address = fault->params.cf_address;
		ptr->params.mask = fault->params.mask;
		ptr->params.instruction = fault->params.instruction;
		ptr->params.set_bit = fault->params.set_bit;
		ptr->is_active = 0;
    ptr->next = 0;

    head = curr = ptr;

    num_list_elements++;

    return ptr;
}

/**
 * Allocates the size for a new entry in the linked list and parses the elements to it.
 *
 * @param[in] fault - all necessary elements for defining a fault, which are held
 *                              by the struct "Fault"
 * @param[out] ptr - pointer to the linked list entry (added element)
 */
static FaultList* add_to_fault_list(struct Fault *fault)
{
    if(head == 0)
        return create_fault_list(fault);

    FaultList *ptr = (FaultList*) malloc( sizeof(struct Fault) );
    if(0 == ptr)
    {
        error_printf("Node creation failed\n");
        return 0;
    }

    ptr->id = fault->id;
		ptr->component = fault->component;
		ptr->target = fault->target;
		ptr->mode = fault->mode;
		ptr->trigger = fault->trigger;
		ptr->timer = fault->timer;
		ptr->type = fault->type;
		ptr->duration = fault->duration;
		ptr->interval = fault->interval;
		ptr->params.address = fault->params.address;
		ptr->params.cf_address = fault->params.cf_address;
		ptr->params.mask = fault->params.mask;
		ptr->params.instruction = fault->params.instruction;
		ptr->params.set_bit = fault->params.set_bit;
		ptr->is_active = 0;
    ptr->next = 0;

    curr->next = ptr;
    curr = ptr;

    num_list_elements++;

    return ptr;
}

#if defined(DEBUG_FAULT_LIST)
/**
 * Prints all entries in the linked list to the standard  out - only for debug purpose.
 */
static void print_fault_list(void)
{
	FaultList *ptr = head;

    error_printf("\n -------Printing list Start------- \n");
    while(ptr != 0)
    {
        error_printf("id [%d] \n",ptr->id);
        error_printf("component [%s] \n",ptr->component);
        error_printf("target [%s] \n",ptr->target);
        error_printf("mode [%s] \n",ptr->mode);
        error_printf("trigger [%s] \n",ptr->trigger);
        error_printf("timer [%s] \n",ptr->timer);
        error_printf("type [%s] \n",ptr->type);
        error_printf("duration [%s] \n",ptr->duration);
        error_printf("interval [%s] \n",ptr->interval);
        error_printf("params.address [%x] \n",ptr->params.address);
        error_printf("params.cf_address [%x] \n",ptr->params.cf_address);
        error_printf("params.mask [%x] \n",ptr->params.mask);
        error_printf("params.instruction [%x] \n",ptr->params.instruction);
        error_printf("params.set_bit [%x] \n",ptr->params.set_bit);
        error_printf("is_active [%d] \n",ptr->is_active);
        ptr = ptr->next;
        error_printf("\n");
    }
    error_printf("\n -------Printing list End------- \n");

    return;
}
#endif

/**
 * Deletes the linked list and all included elements
 */
void delete_fault_list(void)
{
	FaultList *ptr;

	while ( (ptr = head) )
	{
		head = ptr->next;
		if (ptr)
			free(ptr);
	}

    num_list_elements = 0;
}

/**
 * Returns the size of the linked list (included elements)
 *
 * @param[out] num_list_elements - number of included elements
 */
int getNumFaultListElements(void)
{
	return num_list_elements;
}

/**
 * Returns the corresponding FaultList entry to the linked list.
 *
 * @param[in] element - defines  the index of the desired FaultList entry
 * @param[out] fault_element - corresponding pointer to the entry in the linked list.
 */
FaultList* getFaultListElement(int element)
{
	FaultList *ptr = head, *fault_element = 0;
    int index = 0;

    while (ptr != 0)
    {
    	if (element == index)
    		fault_element =  ptr;

    	index++;
    	ptr = ptr->next;
	}

    return fault_element;
}

/**
 * Searches the maximal fault id number in the linked list.
 *
 * @param[out] max_id - the maximal id number
 */
int getMaxIDInFaultList(void)
{
	FaultList *ptr = head;
    int max_id = 0;

    while (ptr != 0)
    {
    	if (ptr->id > max_id)
        	max_id = ptr->id;

    	ptr = ptr->next;
		}

    return max_id;
}

/**
 * Compares the ending of a string with a given ending.
 *
 * @param[in] string - the whole string
 * @param[in] ending - the string containing the postfix
 * @param[out] - 1 if the string contains the  given ending, 0 otherwise
 */
int ends_with(const char *string, const char *ending)
{
	int string_len = strlen(string);
	int ending_len = strlen(ending);

	if ( ending_len > string_len)
		return 0;

	return !strcmp(&string[string_len - ending_len], ending);
}

/**
 * Extracts the ending of the given string and converts
 * the result to an interger value.
 *
 * @param[in] string - the given string
 * @param[out] - the timer value as integer
 */
int timer_to_int(const char *string)
{
	int string_len = strlen(string);
	char timer_string[string_len-1];

	if ( string_len < 3)
		return 0;

	memset(timer_string, '\0', sizeof(timer_string));
	strncpy(timer_string, string, string_len-2);

	return (int) strtol(timer_string, NULL, 10);
}


/**
 * Checks the data types and the content of the parsed XML-parameters
 * for correctness. IMPORTANT: it does not check, if all necessary parameters
 * are defined.
 */
static void validateXMLInput(void)
{
	FaultList *ptr = head;

    while (ptr != 0)
    {
    	if (!ptr->id || ptr->id == -1)
    	{
    		error_printf("fault id is not a positive, real number\n");
    	}

    	if (!ptr->component || !ptr->target || !ptr->mode)
    	{
    		error_printf("component, target or mode is not defined (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->component
    		&& strcmp(ptr->component, "CPU")
    		&& strcmp(ptr->component, "RAM")
    		&& strcmp(ptr->component, "REGISTER") )
    	{
    		error_printf("component has to be \"CPU, REGISTER or RAM\" (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->target
    		&& strcmp(ptr->target, "REGISTER CELL")
    		&& strcmp(ptr->target, "MEMORY CELL")
    		&& strcmp(ptr->target, "CONDITION FLAGS")
    		&& strcmp(ptr->target, "INSTRUCTION EXECUTION")
    		&& strcmp(ptr->target, "INSTRUCTION DECODER")
    		&& strcmp(ptr->target, "ADDRESS DECODER")
		&& strcmp(ptr->target, "PRINT ADDRESSES TO FILE"))
    	{
    		error_printf("target has to be \"REGISTER CELL, MEMORY CELL, "
    				"CONDITION FLAGS, INSTRUCTION EXECUTION, INSTRUCTION DECODER, "
    				"or ADDRESS DECODER\" (fault id: %d)\n", ptr->id);
    	}

	if (ptr->target && !strcmp(ptr->target, "PRINT ADDRESSES TO FILE")){
		profile_ram_addresses = 1;
}

    	if (ptr->mode
    		&& strcmp(ptr->mode, "NEW VALUE")
    		&& strcmp(ptr->mode, "ZF") && strcmp(ptr->mode, "CF")
    		&& strcmp(ptr->mode, "NF") && strcmp(ptr->mode, "QF")
    		&& strcmp(ptr->mode, "VF") && strcmp(ptr->mode, "SF")
    		&& strcmp(ptr->mode, "BIT-FLIP") && strcmp(ptr->mode, "VF")
    		&& strcmp(ptr->mode, "TF0") && strcmp(ptr->mode, "TF1")
    		&& strcmp(ptr->mode, "WDF0") && strcmp(ptr->mode, "WDF1")
    		&& strcmp(ptr->mode, "RDF0") && strcmp(ptr->mode, "RDF1")
    		&& strcmp(ptr->mode, "IRF0") && strcmp(ptr->mode, "IRF1")
    		&& strcmp(ptr->mode, "DRDF0") && strcmp(ptr->mode, "DRDF1")
    		&& strcmp(ptr->mode, "RDF00") && strcmp(ptr->mode, "RDF01")
    		&& strcmp(ptr->mode, "RDF10") && strcmp(ptr->mode, "RDF11")
    		&& strcmp(ptr->mode, "IRF00") && strcmp(ptr->mode, "IRF01")
    		&& strcmp(ptr->mode, "IRF10") && strcmp(ptr->mode, "IRF11")
    		&& strcmp(ptr->mode, "DRDF00") && strcmp(ptr->mode, "DRDF01")
    		&& strcmp(ptr->mode, "DRDF10") && strcmp(ptr->mode, "DRDF11")
    		&& strcmp(ptr->mode, "CFST00") && strcmp(ptr->mode, "CFST01")
    		&& strcmp(ptr->mode, "CFST10") && strcmp(ptr->mode, "CFST11")
    		&& strcmp(ptr->mode, "CFTR00") && strcmp(ptr->mode, "CFTR01")
    		&& strcmp(ptr->mode, "CFTR10") && strcmp(ptr->mode, "CFTR11")
    		&& strcmp(ptr->mode, "CFWD00") && strcmp(ptr->mode, "CFWD01")
    		&& strcmp(ptr->mode, "CFWD10") && strcmp(ptr->mode, "CFWD11")
    		&& strcmp(ptr->mode, "CFRD00") && strcmp(ptr->mode, "CFRD01")
    		&& strcmp(ptr->mode, "CFRD10") && strcmp(ptr->mode, "CFRD11")
    		&& strcmp(ptr->mode, "CFIR00") && strcmp(ptr->mode, "CFIR01")
    		&& strcmp(ptr->mode, "CFIR10") && strcmp(ptr->mode, "CFIR11")
    		&& strcmp(ptr->mode, "CFDR00") && strcmp(ptr->mode, "CFDR01")
    		&& strcmp(ptr->mode, "CFDR10") && strcmp(ptr->mode, "CFDR11")
    		&& strcmp(ptr->mode, "CFDS0W00") && strcmp(ptr->mode, "CFDS0W01")
    		&& strcmp(ptr->mode, "CFDS0W10") && strcmp(ptr->mode, "CFDS0W11")
    		&& strcmp(ptr->mode, "CFDS1W00") && strcmp(ptr->mode, "CFDS1W01")
    		&& strcmp(ptr->mode, "CFDS1W10") && strcmp(ptr->mode, "CFDS1W11")
    		&& strcmp(ptr->mode, "CFDS0R00") && strcmp(ptr->mode, "CFDS0R01")
    		&& strcmp(ptr->mode, "CFDS1R10") && strcmp(ptr->mode, "CFDS1R11"))
    	{
    		error_printf("unknown mode (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->trigger
    		&& strcmp(ptr->trigger, "ACCESS")
    		&& strcmp(ptr->trigger, "TIME")
    		&& strcmp(ptr->trigger, "PC"))
    	{
    		error_printf("trigger has to be \"ACCESS, TIME or PC\" (fault id: %d)\n", ptr->id);
    	}

    	if (!ptr->params.address)
    	{
    		error_printf("fault address is not a number (fault id: %d)\n", ptr->id);
    	}

    	if (!ptr->params.cf_address)
    	{
    		error_printf("fault coupling address is not a number (fault id: %d)\n", ptr->id);
    	}

    	if (!ptr->params.instruction)
    	{
    		error_printf("fault instruction address is not a number (fault id: %d)\n", ptr->id);
    	}

    	if (!ptr->params.mask)
    	{
    		error_printf("fault mask is not a number (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->trigger && (!strcmp(ptr->trigger, "TIME") || !strcmp(ptr->trigger, "ACCESS"))
    		 && ptr->type && strcmp(ptr->type, "PERMANENT")
    		 && strcmp(ptr->type, "TRANSIENT")
    		 && strcmp(ptr->type, "INTERMITTEND"))
    	{
    		error_printf("type has to be \"PERMANENT, TRANSIENT or "
    				"INTERMITTEND\" for time- or access-triggered faults (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->trigger && !strcmp(ptr->trigger, "PC")
    		&& (ptr->params.address == -1 || !ptr->params.address))
    	{
    		error_printf("PC-address has to be defined in the <params>->"
    				"<instruction>-tag or has  to be a positive, real number (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->timer
    		&& !ends_with(ptr->timer, "MS")
    		&& !ends_with(ptr->timer, "US")
    		&& !ends_with(ptr->timer, "NS"))
    	{
    		error_printf("timer has to be a positive, real number in ns, us or"
    				" ms (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->timer
    		&& (ends_with(ptr->timer, "MS")
    		|| ends_with(ptr->timer, "US")
    		|| ends_with(ptr->timer, "NS"))
    		&& !timer_to_int(ptr->timer) )
    	{
    		error_printf("timer has to be a positive, real number in ns, us or"
    				" ms (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->duration
    		&& !ends_with(ptr->duration, "MS")
    		&& !ends_with(ptr->duration, "US")
    		&& !ends_with(ptr->duration, "NS"))
    	{
    		error_printf("duration has to be a positive, real number in ns, us or"
    				" ms (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->duration
    		&& (ends_with(ptr->duration, "MS")
    		|| ends_with(ptr->duration, "US")
    		|| ends_with(ptr->duration, "NS"))
    		&& !timer_to_int(ptr->duration) )
    	{
    		error_printf("duration has to be a positive, real number in ns, us or"
    				" ms (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->interval
    		&& !ends_with(ptr->interval, "MS")
    		&& !ends_with(ptr->interval, "US")
    		&& !ends_with(ptr->interval, "NS"))
    	{
    		error_printf("interval has to be a positive, real number in ns, us or"
    				" ms (fault id: %d)\n", ptr->id);
    	}

    	if (ptr->interval
    		&& (ends_with(ptr->interval, "MS")
    		|| ends_with(ptr->interval, "US")
    		|| ends_with(ptr->interval, "NS"))
    		&& !timer_to_int(ptr->interval) )
    	{
    		error_printf("interval has to be a positive, real number in ns, us or"
    				" ms (fault id: %d)\n", ptr->id);
    	}

    	ptr = ptr->next;
	}
}

/**
 * Parses the fault parameters from the XML file.
 *
 * @param[in] doc - A structure containing the tree created by a parsed
 *                            doc.
 * @param[in] cur - A structure containing a single node.
 */
static void parseFault(xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *key = 0;
	xmlNodePtr grandchild_node;
	FaultList fault = {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, {-1, -1, -1, -1, -1}, 0};

	cur = cur->xmlChildrenNode;
	while (cur != 0)
	{
	    if ( !xmlStrcmp(cur->name, (const xmlChar *) "id") )
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    fault.id =  (int) strtol((char *) key, 0, 10);
				xmlFree(key);
 	    }
	    else if ( !xmlStrcmp(cur->name, (const xmlChar *) "component") )
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    fault.component = strdup((char *) key);
				xmlFree(key);
	    }
	    else if ( !xmlStrcmp(cur->name, (const xmlChar *) "target") )
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    fault.target = strdup((char *) key);
				xmlFree(key);
	    }
	    else if ( !xmlStrcmp(cur->name, (const xmlChar *) "mode") )
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    fault.mode = strdup((char *) key);
				xmlFree(key);
	    }
	    else if ( !xmlStrcmp(cur->name, (const xmlChar *) "trigger") )
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    fault.trigger = strdup((char *) key);
				xmlFree(key);
	    }
	    else if ( !xmlStrcmp(cur->name, (const xmlChar *) "timer") )
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    fault.timer = strdup((char *) key);
				xmlFree(key);
	    }
	    else if ( !xmlStrcmp(cur->name, (const xmlChar *) "type") )
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    fault.type = strdup((char *) key);
				xmlFree(key);
	    }
	    else if ( !xmlStrcmp(cur->name, (const xmlChar *) "duration") )
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    fault.duration = strdup((char *) key);
				xmlFree(key);
	    }
	    else if ( !xmlStrcmp(cur->name, (const xmlChar *) "interval") )
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    fault.interval = strdup((char *) key);
				xmlFree(key);
	    }
	    else if ( !xmlStrcmp(cur->name, (const xmlChar *) "params") )
	    {
	    	grandchild_node =  cur->xmlChildrenNode;
	    	while (grandchild_node != 0)
	    	{
	    		if ( !xmlStrcmp(grandchild_node->name, (const xmlChar *) "address") )
	    		{
	    			key = xmlNodeListGetString(doc, grandchild_node->xmlChildrenNode, 1);
	    			fault.params.address = (int) strtoul((char *) key, 0, 16);
	    			xmlFree(key);
	    		}
	    		if ( !xmlStrcmp(grandchild_node->name, (const xmlChar *) "cf_address") )
	    		{
	    		  key = xmlNodeListGetString(doc, grandchild_node->xmlChildrenNode, 1);
	    			fault.params.cf_address = (int) strtoul((char *) key, 0, 16);
	    			xmlFree(key);
	    		}
	    		else if ( !xmlStrcmp(grandchild_node->name, (const xmlChar *) "mask") )
	    		{
	    		  key = xmlNodeListGetString(doc, grandchild_node->xmlChildrenNode, 1);
	    			fault.params.mask = (int) strtol((char *) key, 0, 16);
	    			xmlFree(key);
	    		}
	    		if ( !xmlStrcmp(grandchild_node->name, (const xmlChar *) "instruction") )
	    		{
	    		  key = xmlNodeListGetString(doc, grandchild_node->xmlChildrenNode, 1);
	    			fault.params.instruction = (int) strtoul((char *) key, 0, 16);
	    			xmlFree(key);
	    		}
	    		if ( !xmlStrcmp(grandchild_node->name, (const xmlChar *) "set_bit") )
	    		{
	    		  key = xmlNodeListGetString(doc, grandchild_node->xmlChildrenNode, 1);
	    		  fault.params.set_bit = (int) strtol((char *) key, 0, 16);
	    			xmlFree(key);
	    		}

	    		grandchild_node = grandchild_node->next;
	    	}
	    }

	    cur = cur->next;
	}

	add_to_fault_list(&fault);

  return;
}

/**
 * Read the XML-file and checks the basic structure of the XML for
 * correctness. Starts the XML-parser.
 *
 * @param[in] filename - The name of the XML-file containing the fault definitions
 */
static bool parse(const char *filename)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(filename);
	if (doc == 0 )
	{
		error_printf("Document not parsed successfully.\n");
		return -1;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == 0)
	{
		error_printf("Empty document\n");
		xmlFreeDoc(doc);
		return -1;
	}


	if (xmlStrcmp(cur->name, (const xmlChar *) "injection"))
	{
		error_printf("Document of the wrong type, root node != injection\n");
		xmlFreeDoc(doc);
		return -1;
	}

	/**
	 * Starting new fault injection experiment -
	 * Deleting current context
	*/
	if (head != 0)
		delete_fault_list();

	destroy_id_array();
	destroy_ops_on_cell();

	cur = cur->xmlChildrenNode;
	while (cur != 0)
	{
		if ( !xmlStrcmp(cur->name, (const xmlChar *) "fault") )
		{
			parseFault(doc, cur);
		}

	cur = cur->next;
	}

	validateXMLInput();
	xmlFreeDoc(doc);
	return true;
}

//
// /**
//  * Read the XML-file and checks the basic structure of the XML for
//  * correctness. Starts the XML-parser.
//  *
//  * @param[in] filename - The name of the XML-file containing the fault definitions
//  */
bool faultReload(const char *filename)
{
    /*
	     * this initialize the library and check potential ABI mismatches
	     * between the version it was compiled for and the actual shared
	     * library used.
	     */
		int max_id = 0;

		/**
		 * Starting new fault injection experiment -
		 * reset timer and statistics
		*/
		fault_injection_controller_initTimer();
		set_num_injected_faults(0);
		set_num_detected_faults(0);
		set_num_injected_faults_ram_trans(0);
		set_num_injected_faults_ram_perm(0);
		set_num_injected_faults_cpu_trans(0);
		set_num_injected_faults_cpu_perm(0);
		set_num_injected_faults_register_trans(0);
		set_num_injected_faults_register_perm(0);

	  if (!parse(filename))
		 	return false;

#if defined(DEBUG_FAULT_LIST)
    print_fault_list();
#endif
		/**
 			* Initialize the context for a new fault injection experiment
			*/
   	max_id = getMaxIDInFaultList();
   	init_id_array(max_id);
	 	init_ops_on_cell(max_id);
	 	xmlCleanupParser();
	 	return true;
}

/**
 * Allocates and initializes the ops_on_memory_cell- and
 * ops_on_register_cell-array.
 *
 * @param[in] ids - the maximal id number.
 */
void init_ops_on_cell(int ids)
{
	int i = 0, j = 0;

	ops_on_memory_cell = malloc(ids * sizeof(int *));
	ops_on_register_cell = malloc(ids * sizeof(int *));

	for (i = 0; i < ids; i++)
	{
		ops_on_memory_cell[i] = malloc(MEMORY_WIDTH * sizeof(int *));
		ops_on_register_cell[i] = malloc(MEMORY_WIDTH * sizeof(int *));
	}

	for (i = 0; i < ids; i++)
	{
		for (j = 0; j < MEMORY_WIDTH; j++)
		{
			ops_on_memory_cell[i][j] = -1;
			ops_on_register_cell[i][j] = -1;
		}
	}
}

/**
 * Deletes the ops_on_memory_cell- and
 * ops_on_register_cell-array.
 */
void destroy_ops_on_cell(void)
{
	int i = 0;

	for(i = 0; i < getMaxIDInFaultList(); i++)
	{
	    free(ops_on_memory_cell[i]);
	 	free(ops_on_register_cell[i]);
	}

	free(ops_on_memory_cell);
	free(ops_on_register_cell);
}

static int64_t timer_value = 0;

/**
 * Returns the elapsed time after loading a fault-config file.
 *
 * @param[out] - the elapsed time as int64
 */
int64_t fault_injection_controller_getTimer(void)
{
	return qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) - timer_value;
}

/**
 * Initializes the timer value after loading a fault-config file (new
 * fault injection experiment) to the current virtual time in QEMU.
 */
void fault_injection_controller_initTimer(void)
{
  timer_value = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
}
