/*HEADER**********************************************************************
 *
 * @brief  The file emulates a mouse cursor movements
 * 
 *END************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "usb_descriptor.h"
#include <usb_hid.h>


#define MAIN_TASK       10
#define  MOUSE_BUFF_SIZE   (4)   /* report buffer size */
#define  REQ_DATA_SIZE     (1)

/** Main_Task called by MQX */
extern void Main_Task(uint32_t param);
TASK_TEMPLATE_STRUCT MQX_template_list[] = { { MAIN_TASK, Main_Task, 2000L, 7L,
		"Main", MQX_AUTO_START_TASK, 0, 0 }, { 0L, 0L, 0L, 0L, 0L, 0L, 0, 0 } };

/* mouse struct */
typedef struct _mouse_variable_struct {
	HID_HANDLE app_handle;
	bool mouse_init;/* flag to check lower layer status*/
	uint8_t rpt_buf[MOUSE_BUFF_SIZE];/*report/data buff for mouse application*/
	USB_CLASS_HID_ENDPOINT ep[HID_DESC_ENDPOINT_COUNT];
	uint8_t app_request_params[2]; /* for get/set idle and protocol requests*/
} MOUSE_GLOBAL_VARIABLE_STRUCT, *PTR_MOUSE_GLOBAL_VARIABLE_STRUCT;


/* Add all the variables needed for mouse.c to this structure */
extern USB_ENDPOINTS usb_desc_ep;
extern DESC_CALLBACK_FUNCTIONS_STRUCT desc_callback;

/* global variable g_mouse */
MOUSE_GLOBAL_VARIABLE_STRUCT g_mouse;

/* buttons used for mouse movement and click */
LWGPIO_STRUCT button_up;
LWGPIO_STRUCT button_down;
LWGPIO_STRUCT button_left;
LWGPIO_STRUCT button_right;
LWGPIO_STRUCT button_click;

/**
 *      @name InitializeIO
 *
 *
 *      @brief Initialization of buttons
 */
void InitializeIO(void)
{

	/* up button (SW4) */
	lwgpio_init(&button_up, BSP_SW4, LWGPIO_DIR_INPUT, LWGPIO_VALUE_NOCHANGE);
	lwgpio_set_functionality(&button_up ,BSP_SW4_MUX_GPIO);
	lwgpio_set_attribute(&button_up, LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_ENABLE);

	/* down button (SW2) */
	lwgpio_init(&button_down,BSP_SW2, LWGPIO_DIR_INPUT, LWGPIO_VALUE_NOCHANGE);
	lwgpio_set_functionality(&button_down ,BSP_SW2_MUX_GPIO);
	lwgpio_set_attribute(&button_down, LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_ENABLE);

	/* left button (SW3) */
	lwgpio_init(&button_left, BSP_SW3, LWGPIO_DIR_INPUT, LWGPIO_VALUE_NOCHANGE);
	lwgpio_set_functionality(&button_left ,BSP_SW3_MUX_GPIO);
	lwgpio_set_attribute(&button_left, LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_ENABLE);

	/* right button (SW1) */
	lwgpio_init(&button_right, BSP_SW1, LWGPIO_DIR_INPUT, LWGPIO_VALUE_NOCHANGE);
	lwgpio_set_functionality(&button_right ,BSP_SW1_MUX_GPIO);
	lwgpio_set_attribute(&button_right, LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_ENABLE);

	/* click button (SW5) */
	lwgpio_init(&button_click, BSP_SW5, LWGPIO_DIR_INPUT, LWGPIO_VALUE_NOCHANGE);
	lwgpio_set_functionality(&button_click ,BSP_SW5_MUX_GPIO);
	lwgpio_set_attribute(&button_click, LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_ENABLE);


}
/*****************************************************************************
 * 
 *      @name     move_mouse
 *
 *      @brief    This function makes the cursor on screen move left,right
 *                up and down, using buttons
 *                Also registers clicks from button_click
 * 
 ******************************************************************************/
void move_mouse(void) {


	/** up mouse movement */
	if (lwgpio_get_value(&button_up) == LWGPIO_VALUE_LOW) {
		g_mouse.rpt_buf[0] = 0;
		g_mouse.rpt_buf[1] = 0;
		g_mouse.rpt_buf[2] = (uint8_t) (-2);
	}

	/** down mouse movement */
	else if(lwgpio_get_value(&button_down) == LWGPIO_VALUE_LOW){
		g_mouse.rpt_buf[0] = 0;
		g_mouse.rpt_buf[1] = 0;
		g_mouse.rpt_buf[2] = 2;
	}

	/** left mouse movement */
	else if(lwgpio_get_value(&button_left) == LWGPIO_VALUE_LOW){
		g_mouse.rpt_buf[0] = 0;
		g_mouse.rpt_buf[1] = (uint8_t) (-2);
		g_mouse.rpt_buf[2] = 0;
	}

	/** right mouse movement */
	else if(lwgpio_get_value(&button_right) == LWGPIO_VALUE_LOW){
		g_mouse.rpt_buf[0] = 0;
		g_mouse.rpt_buf[1] = 2;
		g_mouse.rpt_buf[2] = 0;
	}

	/** left mouse button click */
	else if(lwgpio_get_value(&button_click) == LWGPIO_VALUE_LOW){
		g_mouse.rpt_buf[0] = 1;
		g_mouse.rpt_buf[1] = 0;
		g_mouse.rpt_buf[2] = 0;
	}

	/** no mouse movement */
	else{
		g_mouse.rpt_buf[0] = 0;
		g_mouse.rpt_buf[1] = 0;
		g_mouse.rpt_buf[2] = 0;
	}


	_time_delay(100);

	/** send data about mouse movement/click */
	(void) USB_Class_HID_Send_Data(g_mouse.app_handle, HID_ENDPOINT,
			g_mouse.rpt_buf, MOUSE_BUFF_SIZE);



}

/******************************************************************************
 * 
 *    @name        USB_App_Callback
 *    
 *    @brief       This function handles the callback  
 *                  
 *    @param       handle : handle to Identify the controller
 *    @param       event_type : value of the event
 *    @param       val : gives the configuration value 
 * 
 *    @return      None
 *
 *****************************************************************************/
void USB_App_Callback(uint8_t event_type, void* val, void *arg) {
	UNUSED_ARGUMENT(arg)
	UNUSED_ARGUMENT(val)


	if(event_type == USB_APP_BUS_RESET){
		g_mouse.mouse_init = FALSE;
	}
	else if(event_type == USB_APP_ENUM_COMPLETE){
		/* if enumeration is complete set mouse_init
		   so that application can start */
		g_mouse.mouse_init = TRUE;
	}

	/** start mouse movement */
	if(g_mouse.mouse_init){
		move_mouse();
	}
	return;
}

/******************************************************************************
 * 
 *    @name        USB_App_Param_Callback
 *    
 *    @brief       This function handles the callback for Get/Set report req  
 *                  
 *    @param       request  :  request type
 *    @param       value    :  give report type and id
 *    @param       data     :  pointer to the data 
 *    @param       size     :  size of the transfer
 *
 *    @return      status
 *                  USB_OK  :  if successful
 *                  else return error
 *
 *****************************************************************************/
uint8_t USB_App_Param_Callback(uint8_t request,
		uint16_t value,
		uint8_t **data,
		uint32_t* size,
		void *arg) {

	UNUSED_ARGUMENT(arg)

	uint8_t status = USB_OK;

	/* index == 0 for get/set idle, index == 1 for get/set protocol */
	uint8_t index = (uint8_t) ((request - 2) & USB_HID_REQUEST_TYPE_MASK);


	*size = 0;
	/* handle the class request */
	switch (request) {
	case USB_HID_GET_REPORT_REQUEST:
		*data = &g_mouse.rpt_buf[0]; /* point to the report to send */
		*size = MOUSE_BUFF_SIZE; /* report size */
		break;

	case USB_HID_SET_REPORT_REQUEST:
		for (index = 0; index < MOUSE_BUFF_SIZE; index++) { /* copy the report sent by the host */
			g_mouse.rpt_buf[index] = *(*data + index);
		}
		break;

	case USB_HID_GET_IDLE_REQUEST:
		/* point to the current idle rate */
		*data = &g_mouse.app_request_params[index];
		*size = REQ_DATA_SIZE;
		break;

	case USB_HID_SET_IDLE_REQUEST:
		/* set the idle rate sent by the host */
		g_mouse.app_request_params[index] = (uint8_t) ((value & MSB_MASK) >> HIGH_BYTE_SHIFT);
		break;

	case USB_HID_GET_PROTOCOL_REQUEST:
		/* point to the current protocol code 
		 0 = Boot Protocol
		 1 = Report Protocol*/
		*data = &g_mouse.app_request_params[index];
		*size = REQ_DATA_SIZE;
		break;

	case USB_HID_SET_PROTOCOL_REQUEST:
		/* set the protocol sent by the host 
		 0 = Boot Protocol
		 1 = Report Protocol*/
		g_mouse.app_request_params[index] = (uint8_t) (value);
		break;
	}
	return status;
}

/******************************************************************************
 *  
 *   @name        TestApp_Init
 * 
 *   @brief       This function is the entry for mouse (or other usage)
 * 
 *   @param       None
 * 
 *   @return      None
 **                
 *****************************************************************************/
void TestApp_Init(void) {
	HID_CONFIG_STRUCT config_struct;

	/* initialize the Global Variable Structure */
	USB_mem_zero(&g_mouse, sizeof(MOUSE_GLOBAL_VARIABLE_STRUCT));
	USB_mem_zero(&config_struct, sizeof(HID_CONFIG_STRUCT));

	/* Initialize the USB interface */

	config_struct.ep_desc_data = &usb_desc_ep;
	config_struct.hid_class_callback.callback = USB_App_Callback;
	config_struct.hid_class_callback.arg = &g_mouse.app_handle;
	config_struct.param_callback.callback = USB_App_Param_Callback;
	config_struct.param_callback.arg = &g_mouse.app_handle;
	config_struct.desc_callback_ptr = &desc_callback;
	config_struct.desc_endpoint_cnt = HID_DESC_ENDPOINT_COUNT;
	config_struct.ep = g_mouse.ep;

	if (MQX_OK != _usb_device_driver_install(USBCFG_DEFAULT_DEVICE_CONTROLLER)) {
		printf("Driver could not be installed\n");
		return;
	}

	/** initialize HID */
	g_mouse.app_handle = USB_Class_HID_Init(&config_struct);

	/** initialize IO (buttons) */
	InitializeIO();



}

/*FUNCTION*----------------------------------------------------------------
 * 
 * Function Name  : Main_Task
 * Returned Value : None
 * Comments       :
 *     First function called.  Calls Test_App
 *     callback functions.
 * 
 *END*--------------------------------------------------------------------*/
void Main_Task(uint32_t param) {
	UNUSED_ARGUMENT(param)

	TestApp_Init();


	while (1) {
			/* call the periodic task function */
			USB_HID_Periodic_Task();
		}
}

/* EOF */
