從 wxPython.wx 載入 *
從 code 載入 InteractiveInterpreter

概念 主頁面(wxFrame, InteractiveInterpreter):
    定義 __init__(自己, 上層概念, 代號, 標題):
        wxFrame.__init__(自己, 上層概念, 代號, 標題,
                         wxPoint(150,150), wxSize(600, 400))
	InteractiveInterpreter.__init__(自己, None)

	自己.文字窗  = wxTextCtrl(自己, 30,
		">>> ", size=(600, 400),	style=wxTE_MULTILINE)
	自己.文字窗.SetInsertionPointEnd()
	自己.文字窗.SetFont(wxFont(10, wxMODERN, wxNORMAL, wxNORMAL))
	
	EVT_CHAR(自己.文字窗, 自己.發生按鍵)

	自己.命令行 = ''
	自己.繼續 = 0

    定義 發生按鍵(自己, 事件):
	載入 traceback, code, sys
	鍵 = 事件.GetKeyCode()
	如 鍵 == WXK_RETURN:
		文句 = 自己.文字窗.GetLineText(自己.文字窗.GetNumberOfLines()-1)[4:]
		自己.命令行 += 文句 + '\n'
		如 自己.命令行 == '\n': 
			自己.繪畫('>>> ')
			傳回
		如 自己.繼續 == 1 且 自己.命令行[-2:] != '\n\n': 
			自己.繪畫('... ')
			傳回
		c = 自己.運行(自己.命令行)
		如 (c == 1):
			自己.繪畫('... ')
			自己.繼續 = 1
		否則:
			自己.繼續 = 0
			自己.命令行 = ''
			自己.繪畫('>>> ')

	不然 鍵==WXK_UP 或 鍵==WXK_DOWN:
		忽略

	不然 鍵==WXK_BACK:
		長度 = 自己.文字窗.GetLineLength(自己.文字窗.GetNumberOfLines()-1)
		如 長度 < 5:
			忽略
		否則:
			事件.Skip()
		
	否則:		
		事件.Skip()

    定義 運行(自己, 指令):
	stdout, stderr = sys.stdout, sys.stderr
	sys.stdout, sys.stderr = 輸出接口(自己), 輸出接口(自己)
	結果 = InteractiveInterpreter.runsource(自己, 指令)
	sys.stdout, sys.stderr = stdout, stderr
	傳回 結果

    定義 繪畫(自己, 文字):
	尾字 = 自己.文字窗.GetValue()[-1]
	文字 = 文字.替換('\r\n','\n')
	文字 = 文字.替換('\n\n','\n')
	如 尾字 != '\n':
		自己.文字窗.AppendText('\n')
	如 文字[-1] == '\n': 
		文字 = 文字[:-1]
	自己.文字窗.AppendText(文字)		

概念 輸出接口:
    定義 __init__(自己, 窗):
        自己.窗 = 窗

    定義 write(自己, 文字):
        自己.窗.繪畫(文字)

    定義 writelines(自己, 文字列):
        套用(自己.write, 文字列)

    定義 flush(自己):
        忽略

#---------------------------------------------------------------------------
# 每個 wxWindows 的應用必須是 wxApp 的擴展概念
概念 我的程式(wxApp):
    # wxWindows 會自動呼叫這個函數作啟動用
    定義 OnInit(自己):
        畫面 = 主頁面(NULL, wxNewId(), '''中蟒互動環境 (pre-Alpha)''')
        畫面.Show(true)
        自己.SetTopWindow(畫面)
        傳回 true

#---------------------------------------------------------------------------
# 非常短的主迴圈
應用程式 = 我的程式(0)  # 創建一個 '我的程式' 概念的實體
應用程式.MainLoop()     # 開始運行
