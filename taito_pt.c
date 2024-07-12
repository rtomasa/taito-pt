#include <linux/module.h>
#include <linux/usb.h>
#include <linux/hid.h>
#include <linux/input.h>

#define USB_TAITO_VENDOR_ID 0x0ae4
#define USB_TAITO_PRODUCT_ID 0x0701

static struct usb_device_id taito_table[] = {
    { USB_DEVICE(USB_TAITO_VENDOR_ID, USB_TAITO_PRODUCT_ID) },
    { }
};
MODULE_DEVICE_TABLE(usb, taito_table);

struct taito_device {
    struct input_dev *input;
    struct usb_device *udev;
    struct usb_interface *interface;
    unsigned char *data;
    dma_addr_t data_dma;
    struct urb *irq;
};

static void taito_irq(struct urb *urb) {
    struct taito_device *taito = urb->context;
    unsigned char *data = taito->data;
    struct input_dev *input = taito->input;
    int retval;

    switch (urb->status) {
        case 0:
            // Success
            break;
        case -ECONNRESET:
        case -ENOENT:
        case -ESHUTDOWN:
            // Unplugged
            return;
        default:
            goto resubmit;
    }

    // Report buttons
    input_report_key(input, BTN_LEFT, data[0] & 0x80);
    input_report_key(input, BTN_RIGHT, data[1] & 0x01);
    input_report_key(input, BTN_MIDDLE, data[0] & 0x10);
    input_report_key(input, BTN_SIDE, data[0] & 0x02);
    input_report_key(input, BTN_EXTRA, data[0] & 0x04);
    // Report trackball
    input_report_rel(input, REL_HWHEEL, (signed char)data[2] * 16);
    input_report_rel(input, REL_WHEEL, (signed char)data[3] * 16);
    // Report paddle
    input_report_rel(input, REL_HWHEEL, -(signed char)data[4] * 8);

    input_sync(input);

resubmit:
    retval = usb_submit_urb(urb, GFP_ATOMIC);
    if (retval)
        dev_err(&taito->interface->dev, "usb_submit_urb failed with result %d\n", retval);
}

static int taito_open(struct input_dev *input) {
    struct taito_device *taito = input_get_drvdata(input);
    taito->irq->dev = taito->udev;
    if (usb_submit_urb(taito->irq, GFP_KERNEL))
        return -EIO;
    return 0;
}

static void taito_close(struct input_dev *input) {
    struct taito_device *taito = input_get_drvdata(input);
    usb_kill_urb(taito->irq);
}

static int taito_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    struct usb_device *udev = interface_to_usbdev(interface);
    struct taito_device *taito;
    struct input_dev *input;
    int pipe, maxp;
    int error = -ENOMEM;

    taito = kzalloc(sizeof(struct taito_device), GFP_KERNEL);
    input = input_allocate_device();
    if (!taito || !input)
        goto fail1;

    taito->udev = udev;
    taito->interface = interface;
    taito->input = input;

    input_set_drvdata(input, taito);

    input->name = "TAITO USB Paddle & Trackball Controller";
    input->phys = "usb/input0";
    input->id.bustype = BUS_USB;
    input->id.vendor = USB_TAITO_VENDOR_ID;
    input->id.product = USB_TAITO_PRODUCT_ID;
    input->id.version = 0x0100;
    input->dev.parent = &interface->dev;

    input->open = taito_open;
    input->close = taito_close;

    input_set_capability(input, EV_KEY, BTN_LEFT);
    input_set_capability(input, EV_KEY, BTN_RIGHT);
    input_set_capability(input, EV_KEY, BTN_MIDDLE);
    input_set_capability(input, EV_KEY, BTN_SIDE);
    input_set_capability(input, EV_KEY, BTN_EXTRA);
    input_set_capability(input, EV_REL, REL_WHEEL);
    input_set_capability(input, EV_REL, REL_HWHEEL);

    taito->data = usb_alloc_coherent(udev, 8, GFP_ATOMIC, &taito->data_dma);
    if (!taito->data)
        goto fail2;

    taito->irq = usb_alloc_urb(0, GFP_KERNEL);
    if (!taito->irq)
        goto fail3;

    pipe = usb_rcvintpipe(udev, 1);
    maxp = usb_maxpacket(udev, pipe);
    usb_fill_int_urb(taito->irq, udev, pipe, taito->data, maxp > 8 ? 8 : maxp,
                     taito_irq, taito, 10);
    taito->irq->transfer_dma = taito->data_dma;
    taito->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    error = input_register_device(taito->input);
    if (error)
        goto fail4;

    usb_set_intfdata(interface, taito);

    pr_info("TAITO USB Paddle & Trackball Controller plugged\n");
    return 0;

fail4:
    usb_free_urb(taito->irq);
fail3:
    usb_free_coherent(udev, 8, taito->data, taito->data_dma);
fail2:
    input_free_device(input);
fail1:
    kfree(taito);
    return error;
}

static void taito_disconnect(struct usb_interface *interface) {
    struct taito_device *taito = usb_get_intfdata(interface);

    usb_set_intfdata(interface, NULL);
    if (taito) {
        usb_kill_urb(taito->irq);
        input_unregister_device(taito->input);
        usb_free_urb(taito->irq);
        usb_free_coherent(taito->udev, 8, taito->data, taito->data_dma);
        kfree(taito);
    }

    pr_info("TAITO USB Paddle & Trackball Controller unplugged\n");
}

static struct usb_driver taito_pt = {
    .name = "taito_pt",
    .id_table = taito_table,
    .probe = taito_probe,
    .disconnect = taito_disconnect,
};

module_usb_driver(taito_pt);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("(RTA) <ruben.tomas.alonso@gmail.com>");
MODULE_DESCRIPTION("TAITO USB Paddle & Trackball Controller");
