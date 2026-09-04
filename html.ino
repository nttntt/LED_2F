/* HTMLページ */
const String strHtmlHeader = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html { font-family: Helvetica; display: inline-block; margin: 0px auto;text-align: center;} 
      h1 {font-size:28px;} 
      body {text-align: center;} 
      table { border-collapse: collapse; margin-left:auto; margin-right:auto; }
      th { padding: 12px; background-color: #0000cd; color: white; border: solid 2px #c0c0c0; }
      tr { border: solid 2px #c0c0c0; padding: 12px; }
      td { border: solid 2px #c0c0c0; padding: 12px; }
      .value { color:blue; font-weight: bold; padding: 1px;}
    </style>
  </head>
)rawliteral";
const String strHtmlBody = R"rawliteral(
  <body>
    <h1>%PAGE_TITLE%</h1>
    <table><tr><th>ELEMENT</th><th>VALUE</th></tr>
    <tr><td>Pattern</td><td><span class="value">%PATTERN%</span></td></tr>
    <tr><td>Brightness</td><td><span class="value">%BRIGHTNESS%</span></td></tr>
    <tr><td>Sensitivity</td><td><span class="value">%SENSITIVITY%</span></td></tr>
    </table>
    <input type="button" value="Mode" style="font-size:32px;" onclick="location.href='/?button=mode';"><br>
    <input type="button" value="^" style="font-size:32px;" onclick="location.href='/?button=up';"><br>
    <input type="button" value="<" style="font-size:32px;" onclick="location.href='/?button=left';">
    <input type="button" value=">" style="font-size:32px;" onclick="location.href='/?button=right';"><br>
    <input type="button" value="V" style="font-size:32px;" onclick="location.href='/?button=down';">

  </body></html>
)rawliteral";

/* HTTP レスポンス処理 */
void httpSendResponse(void) {
  String strHtml = strHtmlHeader + strHtmlBody;
  char numStr[10];
  strHtml.replace("%PAGE_TITLE%", LEDS_NAME);
  sprintf(numStr, "%d", gCurrentPatternNumber);
  strHtml.replace("%PATTERN%", numStr);
  sprintf(numStr, "%d", gBrightness);
  strHtml.replace("%BRIGHTNESS%", numStr);
  sprintf(numStr, "%d", gSensitivity);
  strHtml.replace("%SENSITIVITY%", numStr);
  // HTMLを出力する
  server.send(200, "text/html", strHtml);
}

/* HTTP リクエスト処理 */
void handleHtml(void) {

  // 「/?button=○」のパラメータが指定されているかどうかを確認
  if (server.hasArg("button")) {
    // パラメータに応じて、LEDを操作
    if (server.arg("button").equals("mode")) {
      nextPattern();
    } else if (server.arg("button").equals("up")) {
      gY=-2000;
    } else if (server.arg("button").equals("left")) {
      gX=-2000;
    } else if (server.arg("button").equals("right")) {
      gX=2000;
    } else if (server.arg("button").equals("down")) {
      gY=2000;
    }
  }
  // ページ更新
  httpSendResponse();
}

// 存在しないアドレスが指定された時の処理
void handleNotFound(void) {
  server.send(404, "text/plain", "Not Found.");
}
