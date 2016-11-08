(function($) {
  var columnDefs = [
    {headerName: "Customer", field: "display_name"},
    {headerName: "Company", field: "company"},
    {headerName: "Contact", field: "contact_number"}
  ];

  var rowData = [
    {display_name: "Toyota", company: "Celica", contact_number: 35000},
    {display_name: "Ford", company: "Mondeo", contact_number: 32000},
    {display_name: "Porsche", company: "Boxter", contact_number: 72000}
  ];

  var gridOptions = {
    columnDefs: columnDefs,
    rowData: rowData
  };

// wait for the document to be loaded, otherwise
// ag-Grid will not find the div in the document.
  $(document).ready(function() {
    var eGridDiv = document.querySelector('#myGrid');
    new agGrid.Grid(eGridDiv, gridOptions);
  });
})(jQuery);
