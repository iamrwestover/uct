Dear <?php print $vendor_name; ?>,

Please find our purchase order details below.

<?php print check_plain($po->message); ?>
<?php print $po_table; ?>

Thanks,
<?php print $site_name; ?>