```javascript
/*
// Author: Trieu Le
// Email: lethanhtrieuk36@gmail.com
// Publish date: 07-Oct-2015
// Description: This code for demonstration send data from ESP8266 into Google Spreadsheet
// GET request syntax:
// https://script.google.com/macros/s/<gscript id>/exec?header_here=data_here
// Modifyed by Yhan Silva for Engenheiro Caiçara: http://engenheirocaicara.com data: 27/10/2018
*/



function doGet(e) { 
  Logger.log( JSON.stringify(e) );  // Visualiza parametros
  
  var result = 'Ok'; // Sucesso
  if (e.parameter == 'undefined') {
    result = 'Sem Parâmetros';
  }
  else {
    var sheet_id = '1xcRlbSfyknbatq29mph8NVgVaWSLXA4r3PXJCyBgpOo';      // Insira o ID da sua planilha
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();     // obtêm a planilha ativa através do ID
    var newRow = sheet.getLastRow() + 1;                        
    var rowData = [];
    rowData[0] = new Date();                                            // TimeStamp na coluna A
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      
      // -- Condição para inserir parâmetros na planilha
      
      switch (param) {
        case 'TEMPERATURA': //Parametro deverá ser o título da coluna
          rowData[1] = value; //Inclui valor na coluna B
          result = 'Dado inserido na coluna B';
          break;
        case 'UMIDADE': //Parametro deverá ser o título da coluna
          rowData[2] = value; //Inclui valor na coluna C
          result += ' ,Dado inserido na coluna C';
          break;  
        case 'CO2': //Parametro deverá ser o título da coluna
          rowData[3] = value; //Inclui valor na coluna D
          result += ' ,Dado inserido na coluna D';
        default:
          result = "Parâmetro inexistente/não encontrado!";
      }
    }
    Logger.log(JSON.stringify(rowData));
    
    // Escreve nova linha na planilha
    
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
  }
  // Return result of operation
  return ContentService.createTextOutput(result);
}
// -- Remove aspas simples e duplas --

function stripQuotes( value ) {
  return value.replace(/^["']|['"]$/g, "");
}


