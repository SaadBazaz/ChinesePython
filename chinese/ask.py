#--BIG5--

回答 = 讀入('''你認為中文程式語言有存在價值嗎 ? (有/沒有)''')

寫 回答
寫 回答.字串編碼()
寫 回答.十六進()

如 回答 == '''有'''.調整編碼():
	寫 '''好吧, 讓我們一起努力!'''
不然 回答 == '''沒有'''.調整編碼():
	寫 '''好吧,中文並沒有作為程式語言的價值.'''
否則:
	寫 '''請認真考慮後再回答.'''
