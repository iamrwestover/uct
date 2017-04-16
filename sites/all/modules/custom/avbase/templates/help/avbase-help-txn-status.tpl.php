<h2 id="help-txn-status"><?php print $help_title; ?></h2>
<?php
  $tsd = avtxns_txn_status_details();
?>

<table class="uk-table uk-table-condensed">
  <tr>
    <td class="uk-width-2-10">
      <span class="uk-badge <?php print $tsd[1]['badge_class']; ?>"><i class="<?php print 'uk-icon-justify uk-icon-' . $tsd[1]['badge_icon']; ?>"></i></span>
      <span class="uk-text-uppercase"><?php print $tsd[1]['title']; ?></span>
    </td>
    <td class="uk-width-8-10">
      This is the default status. Transactions remain OPEN as long as they are not yet referenced to and from another transaction.
      <br />Only OPEN transactions can be edited.
    </td>
  </tr>
  <tr>
    <td>
      <span class="uk-badge <?php print $tsd[0]['badge_class']; ?>"><i class="<?php print 'uk-icon-justify uk-icon-' . $tsd[0]['badge_icon']; ?>"></i></span>
      <span class="uk-text-uppercase"><?php print $tsd[0]['title']; ?></span>
    </td>
    <td>
      Transactions are LOCKED when they are referenced to and from another transaction.
      <br />For example, an Invoice will be LOCKED if a payment is received for it.
      <br />Transactions that can be credited (e.g., sales return, credit memo, etc.) will be LOCKED if some of its available credit is applied on another transaction.
    </td>
  </tr>
  <tr>
    <td>
      <span class="uk-badge <?php print $tsd[4]['badge_class']; ?>"><i class="<?php print 'uk-icon-justify uk-icon-' . $tsd[4]['badge_icon']; ?>"></i></span>
      <span class="uk-text-uppercase"><?php print $tsd[4]['title']; ?></span>
    </td>
    <td>
      Transactions are CLOSED when it cannot be used for reference on any future transactions.
      <br />For example, an invoice that is fully paid will be automatically CLOSED.<br />Transactions that can be credited (e.g., sales return, credit memo, etc.) will be CLOSED when there are no more available credits in it.
    </td>
  </tr>
  <tr>
    <td>
      <span class="uk-badge <?php print $tsd[2]['badge_class']; ?>"><i class="<?php print 'uk-icon-justify uk-icon-' . $tsd[2]['badge_icon']; ?>"></i></span>
      <span class="uk-text-uppercase"><?php print $tsd[2]['title']; ?></span>
    </td>
    <td>
      Some transactions can be sent for approval.
      <br />For example, deliveries that exceed the configured limit and check payments are temporarily set to PENDING status.
      <br />Users with authorized permissions will be able to approve PENDING transactions.
    </td>
  </tr>
  <tr>
    <td>
      <span class="uk-badge <?php print $tsd[3]['badge_class']; ?>"><i class="<?php print 'uk-icon-justify uk-icon-' . $tsd[3]['badge_icon']; ?>"></i></span>
      <span class="uk-text-uppercase"><?php print $tsd[3]['title']; ?></span>
    </td>
    <td>
      A cancelled transaction is marked as VOID. Only OPEN transactions can be voided. Voiding a transaction will unlock and/or re-open related transactions.
    </td>
  </tr>
</table>
