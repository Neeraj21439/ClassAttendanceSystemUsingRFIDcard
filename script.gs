function doGet(e) { 
  Logger.log( JSON.stringify(e) );
  var result = 'Ok';
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
  }
  else {
    var sheet_id = 'SHEET ID'; 	// Spreadsheet ID
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();
    var newRow = sheet.getLastRow() + 1;                        
    var rowData = [];

    var Curr_Date = Utilities.formatDate(new Date(), "Asia/Kolkata", 'yyyy-MM-dd'); // Correct Date Format
    var Curr_Time = Utilities.formatDate(new Date(), "Asia/Kolkata", 'HH:mm:ss');
    
    rowData[2] = Curr_Date; // Date in column C
    rowData[3] = Curr_Time; // Time in column D
    
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'name':
          rowData[0] = value; // Name in column A
          result = 'Name Written on column A'; 
          break;
        case 'rollno':
          rowData[1] = value; // Roll Number in column B
          result = 'Roll Number Written on column B';
          break;
        default:
          result = "unsupported parameter";
      }
    }
    Logger.log(JSON.stringify(rowData));
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
  }
  return ContentService.createTextOutput(result);
}
function stripQuotes( value ) {
  return value.replace(/^['"]|['"]$/g, "");
}
