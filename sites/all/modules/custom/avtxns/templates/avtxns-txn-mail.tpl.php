Dear <?php print $client_name; ?>,

Please find our purchase order details below.

<?php print check_plain($transaction->message); ?>
<?php print $transaction_table; ?>

Thanks,
<?php print $site_name; ?>