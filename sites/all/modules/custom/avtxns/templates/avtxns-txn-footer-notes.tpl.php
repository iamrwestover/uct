<p>
  <div class="uk-grid uk-grid-small no-page-break-inside">
    <?php if (arg(1) == AVTXNS_TXN_TYPE_DELIVERY): ?>
      <div class="uk-width-1-1">Received the above items in good order and condition</div>
    <?php endif; ?>
    <div class="uk-width-1-1">&nbsp;</div>
    <div class="uk-width-1-2"><hr style="border-color: #000;" /></div>
    <div class="uk-width-1-2"><hr style="border-color: #000;" /></div>
    <div class="uk-width-1-2 uk-text-center">Signature Over Printed Name</div>
    <div class="uk-width-1-2 uk-text-center">Date</div>

    <?php if (arg(1) == AVTXNS_TXN_TYPE_DELIVERY): ?>
      <div class="uk-width-1-1 uk-margin-top">
        <span class="uk-text-bold">IMPORTANT:</span>
        <br />
        Count goods carefully before signing. Complaints on defective delivery will not be entertained unless the same are noted and informed herein by the hauler's representative.
      </div>
    <?php endif; ?>
  </div>
</p>