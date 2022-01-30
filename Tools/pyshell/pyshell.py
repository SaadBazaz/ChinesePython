�q wxPython.wx ���J *
�q code ���J InteractiveInterpreter

���� �D����(wxFrame, InteractiveInterpreter):
    �w�q __init__(�ۤv, �W�h����, �N��, ���D):
        wxFrame.__init__(�ۤv, �W�h����, �N��, ���D,
                         wxPoint(150,150), wxSize(600, 400))
	InteractiveInterpreter.__init__(�ۤv, None)

	�ۤv.��r��  = wxTextCtrl(�ۤv, 30,
		">>> ", size=(600, 400),	style=wxTE_MULTILINE)
	�ۤv.��r��.SetInsertionPointEnd()
	�ۤv.��r��.SetFont(wxFont(10, wxMODERN, wxNORMAL, wxNORMAL))
	
	EVT_CHAR(�ۤv.��r��, �ۤv.�o�ͫ���)

	�ۤv.�R�O�� = ''
	�ۤv.�~�� = 0

    �w�q �o�ͫ���(�ۤv, �ƥ�):
	���J traceback, code, sys
	�� = �ƥ�.GetKeyCode()
	�p �� == WXK_RETURN:
		��y = �ۤv.��r��.GetLineText(�ۤv.��r��.GetNumberOfLines()-1)[4:]
		�ۤv.�R�O�� += ��y + '\n'
		�p �ۤv.�R�O�� == '\n': 
			�ۤv.ø�e('>>> ')
			�Ǧ^
		�p �ۤv.�~�� == 1 �B �ۤv.�R�O��[-2:] != '\n\n': 
			�ۤv.ø�e('... ')
			�Ǧ^
		c = �ۤv.�B��(�ۤv.�R�O��)
		�p (c == 1):
			�ۤv.ø�e('... ')
			�ۤv.�~�� = 1
		�_�h:
			�ۤv.�~�� = 0
			�ۤv.�R�O�� = ''
			�ۤv.ø�e('>>> ')

	���M ��==WXK_UP �� ��==WXK_DOWN:
		����

	���M ��==WXK_BACK:
		���� = �ۤv.��r��.GetLineLength(�ۤv.��r��.GetNumberOfLines()-1)
		�p ���� < 5:
			����
		�_�h:
			�ƥ�.Skip()
		
	�_�h:		
		�ƥ�.Skip()

    �w�q �B��(�ۤv, ���O):
	stdout, stderr = sys.stdout, sys.stderr
	sys.stdout, sys.stderr = ��X���f(�ۤv), ��X���f(�ۤv)
	���G = InteractiveInterpreter.runsource(�ۤv, ���O)
	sys.stdout, sys.stderr = stdout, stderr
	�Ǧ^ ���G

    �w�q ø�e(�ۤv, ��r):
	���r = �ۤv.��r��.GetValue()[-1]
	��r = ��r.����('\r\n','\n')
	��r = ��r.����('\n\n','\n')
	�p ���r != '\n':
		�ۤv.��r��.AppendText('\n')
	�p ��r[-1] == '\n': 
		��r = ��r[:-1]
	�ۤv.��r��.AppendText(��r)		

���� ��X���f:
    �w�q __init__(�ۤv, ��):
        �ۤv.�� = ��

    �w�q write(�ۤv, ��r):
        �ۤv.��.ø�e(��r)

    �w�q writelines(�ۤv, ��r�C):
        �M��(�ۤv.write, ��r�C)

    �w�q flush(�ۤv):
        ����

#---------------------------------------------------------------------------
# �C�� wxWindows �����Υ����O wxApp ���X�i����
���� �ڪ��{��(wxApp):
    # wxWindows �|�۰ʩI�s�o�Ө�Ƨ@�Ұʥ�
    �w�q OnInit(�ۤv):
        �e�� = �D����(NULL, wxNewId(), '''������������ (pre-Alpha)''')
        �e��.Show(true)
        �ۤv.SetTopWindow(�e��)
        �Ǧ^ true

#---------------------------------------------------------------------------
# �D�`�u���D�j��
���ε{�� = �ڪ��{��(0)  # �Ыؤ@�� '�ڪ��{��' ����������
���ε{��.MainLoop()     # �}�l�B��
