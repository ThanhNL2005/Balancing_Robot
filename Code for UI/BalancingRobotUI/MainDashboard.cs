using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using ZedGraph;
using Label = System.Windows.Forms.Label;
using System.IO;
using System.Net.Sockets;
using OfficeOpenXml;
using OfficeOpenXml.Style;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using static OfficeOpenXml.ExcelErrorValue;


namespace BalancingRobotUI
{
    public partial class MainDashboard : Form
    {
        /*************** Tao cac doi tuong tren giao dien ***************/

        // Tao form con
        RobotSystemConfig frmRobotSysChild;

        // Auto resize cac phan tu tren giao dien khi Form thay doi kich co
        Dictionary<Control, Rectangle> originalBounds = new Dictionary<Control, Rectangle>();
        Size originalFormSize;
        Size originalParentControlSize;

        // Dinh nghia toan cuc Frame du lieu nhan tu ESP32
        private const byte UPLINK_START_BYTE = 0xAA;
        private const byte END_BYTE = 0xED;
        private int structSize = Marshal.SizeOf(typeof(DataReceive));
        private List<byte> serialBuffer = new List<byte>();

        // Bo loc thong thap bac 2, he so gain = 1 (Second-order unity-gain low pass filter)
        private LowPassFilter filterDisplayData = new LowPassFilter();
        private double latestFilteredValue = 0;
        private double dataRecvSampTime = 0.5;    // Received data sampling time (Ts):
        private ushort cutoffFrq = 20;             // Cutoff frequency of LPF (Fc): 

        // Vung chia Panel
        private Panel pnData = new Panel();
        private Panel pnDesigner = new Panel();
        private Panel pnControl = new Panel();
        private Panel pnGraph = new Panel();
        private Panel pnGraphChoice = new Panel();

        // Timer
        System.Windows.Forms.Timer tmrUpdateData = new System.Windows.Forms.Timer();
        int tickStart = 0;
        private bool isConnected = false; // Đã kết nối với cổng COM = true, đã ngắt kết nối cổng COM = false

        // Bieu do Zedgraph
        ZedGraphControl zgc1 = new ZedGraphControl();
        ZedGraphControl zgc2 = new ZedGraphControl();
        ZedGraphControl zgc3 = new ZedGraphControl();
        ZedGraphControl zgc4 = new ZedGraphControl();
        ZedGraphControl zgc5 = new ZedGraphControl();
        ZedGraphControl zgc6 = new ZedGraphControl();
        ZedGraphControl zgc7 = new ZedGraphControl();
        ZedGraphControl zgc8 = new ZedGraphControl();
        List<ZedGraphControl> zedGraphControlsList;

        // Khoi tao bieu do
        GraphPane mainDisplayPane = new GraphPane();
        GraphPane xAnglePane = new GraphPane();
        GraphPane zAnglePane = new GraphPane();
        GraphPane xAngularVelocityPane = new GraphPane();
        GraphPane zAngularVelocityPane = new GraphPane();
        GraphPane xCoordinatePane = new GraphPane();
        GraphPane leftTorquePane = new GraphPane();
        GraphPane rightTorquePane = new GraphPane();

        // Danh sach diem tren do thi zedgraph PairPointList
        PointPairList xAnglePoints = new PointPairList();
        PointPairList zAnglePoints = new PointPairList();
        PointPairList xAngularVelocityPoints = new PointPairList();
        PointPairList zAngularVelocityPoints = new PointPairList();
        PointPairList xCoordinatePoints = new PointPairList();
        PointPairList leftTorquePoints = new PointPairList();
        PointPairList rightTorquePoints = new PointPairList();

        LineItem xAngleLine;
        LineItem zAngleLine;
        LineItem xAngularVelocityLine;
        LineItem zAngularVelocityLine;
        LineItem xCoordinateLine;
        LineItem leftTorqueLine;
        LineItem rightTorqueLine;

        /**** Nut an Button ****/
        Button btnConnect;
        Button btnDisconnect;
        Button btnRun;
        Button btnSaveData;
        Button btnExit;

        Button btnRobotRun;
        Button btnRobotStop;
        Button btnRobotSysConfig;

        Button btnHelp;
        System.Windows.Forms.Label lblHelp;

        /**** O noi dung TextBox va nhan Label ****/
        // Du lieu hien thi
        TextBox txtXAngle = new TextBox() { Size = new Size(70, 20), Location = new Point(200, 30) };
        System.Windows.Forms.Label lblXAngle = new System.Windows.Forms.Label() { Text = "Góc x (độ)", Location = new Point(30, 30) };

        TextBox txtZAngle = new TextBox() { Size = new Size(70, 20), Location = new Point(200, 60) };
        System.Windows.Forms.Label lblZAngle = new System.Windows.Forms.Label() { Text = "Góc z (độ)", Location = new Point(30, 60) };

        TextBox txtXAngularVelocity = new TextBox() { Size = new Size(70, 20), Location = new Point(200, 90) };
        System.Windows.Forms.Label lblXAngularVelocity = new System.Windows.Forms.Label() { Text = "Tốc độ góc x (độ/s)", Location = new Point(30, 90) };

        TextBox txtZAngularVelocity = new TextBox() { Size = new Size(70, 20), Location = new Point(200, 120) };
        System.Windows.Forms.Label lblZAngularVelocity = new System.Windows.Forms.Label() { Text = "Tốc độ góc z (độ/s)", Location = new Point(30, 120) };

        TextBox txtXCoordinate = new TextBox() { Size = new Size(70, 20), Location = new Point(200, 150) };
        System.Windows.Forms.Label lblXCoordinate = new System.Windows.Forms.Label() { Text = "Toạ độ x (m)", Location = new Point(30, 150) };

        TextBox txtLeftTorque = new TextBox() { Size = new Size(70, 20), Location = new Point(200, 180) };
        System.Windows.Forms.Label lblLeftTorque = new System.Windows.Forms.Label() { Text = "Momen bánh trái (Nm)", Location = new Point(30, 180) };

        TextBox txtRightTorque = new TextBox() { Size = new Size(70, 20), Location = new Point(200, 210) };
        System.Windows.Forms.Label lblRightTorque = new System.Windows.Forms.Label() { Text = "Momen bánh phải (Nm)", Location = new Point(30, 210) };

        // Thong tin phien ban cap nhat giao dien va nguoi thiet ke
        System.Windows.Forms.Label lblDesigner = new System.Windows.Forms.Label();


        /**** Bang chon GroupBox ****/
        GroupBox grpGraphChoice = new GroupBox();
        GroupBox grpGraphViewMode = new GroupBox();
        GroupBox grpDataView = new GroupBox();
        GroupBox grpCommunication = new GroupBox();
        GroupBox grpUploadControls = new GroupBox();
        GroupBox grpDownloadControls = new GroupBox();

        /**** Nut an RadioButon ****/
        RadioButton radXAngle = new RadioButton();
        RadioButton radZAngle = new RadioButton();
        RadioButton radXAngularVelocity = new RadioButton();
        RadioButton radZAngularVelocity = new RadioButton();
        RadioButton radXCoordinate = new RadioButton();
        RadioButton radLeftTorque = new RadioButton();
        RadioButton radRightTorque = new RadioButton();

        RadioButton radMeshView = new RadioButton();
        RadioButton radAllValuesView = new RadioButton();
        //RadioButton radVehicleView = new RadioButton();

        /**** Thanh tien trinh ProgressBar ****/
        ProgressBar prbCompile = new ProgressBar() { Size = new Size(100, 15), Location = new Point(1300, 70) };

        /**** Cong ket noi Serial Ports ****/
        public SerialPort serCOM;

        /**** Hop lua chon ComboBox va nhan Label ****/
        public ComboBox cmbSerialPort = new ComboBox();
        System.Windows.Forms.Label lblSerialPort = new System.Windows.Forms.Label();
        public ComboBox cmbBaudRate = new ComboBox();
        System.Windows.Forms.Label lblBaudRate = new System.Windows.Forms.Label();
        TextBox txtRunTime = new TextBox() { Size = new Size(70, 20), Location = new Point(200, 90) };
        System.Windows.Forms.Label lblRunTime = new System.Windows.Forms.Label() { Text = "Thời gian chạy đồ thị (s)", Location = new Point(30, 90) };

        string[] serialPortList = {"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "COM10", "COM11", "COM12", "COM13", "COM14", "COM15",
            "COM16", "COM17", "COM18", "COM19", "COM20", "COM21", "COM22", "COM23", "COM24", "COM25", "COM26", "COM27", "COM28", "COM29", "COM30"};
        string[] baudRateList = { "300", "600", "1200", "2400", "4800", "9600", "14400", "19200", "28800", "31250", "38400",
            "57600", "74880", "115200", "230400", "250000", "460800", "500000", "921600", "1000000", "2000000" };

        // Thanh cong cu MenuStrip ****/
        MenuStrip mnuStrip = new MenuStrip();
        ToolStripMenuItem fileToolStrip = new ToolStripMenuItem() { Text = "File" };
        ToolStripMenuItem saveToolStrip = new ToolStripMenuItem() { Text = "Save" };
        ToolStripMenuItem editToolStrip = new ToolStripMenuItem() { Text = "Edit" };
        ToolStripMenuItem deleteToolStrip = new ToolStripMenuItem() { Text = "Delete" };
        ToolStripMenuItem selectToolStrip = new ToolStripMenuItem() { Text = "Select" };
        ToolStripMenuItem newToolStrip = new ToolStripMenuItem() { Text = "New" };
        ToolStripMenuItem historyToolStrip = new ToolStripMenuItem() { Text = "History" };

        /**** Cua so luu tep du lieu SaveFileDialog ****/
        SaveFileDialog saveFileDialog = new SaveFileDialog();

        /**** Hinh anh ****/
        PictureBox picVehicle = new PictureBox();

        /**** Định nghĩa frame lệnh gửi cho xe ****/
        byte downlink_startByte = 0xBB; // byte bắt đầu
        byte endByte = 0xED; // byte kết thúc

        /**** Cau truc du lieu nhan tu ESP32 ****/
        [StructLayout(LayoutKind.Sequential, Pack = 1)] // sắp xếp trật tự các biến và ko có PADDING
        struct DataReceive
        {
            public byte startCheckValue;
            public short xAngle;
            public short zAngle;
            public short xAngularVelocity;
            public short zAngularVelocity;
            public short xCoordinate;
            public short leftTorque;
            public short rightTorque;
            public short checksumCRC16;
            public byte endCheckValue;
        }
        static DataReceive data;
        /***********************************************************************************/


        /************************ Lap trinh hoat dong cho giao dien ************************/
        public MainDashboard()
        {
            InitializeComponent();
            CreateButtonGroups();
            CreateRadioButtonGroups();
            CreatePanels();
            CreateZedGraphGroups();
            CreateLabelAndTextboxGroups();
            CreateMenuStripGroups();
            CreateComboBoxGroups();
            CreatePictureBox();
            CreateSerialPort();
            TimerUpdateDataSetup();
            FormatAllControls(this);

            lblDesigner.Text = "Update: Ver 2.0.25.0525 \nDesigned by: 2WBMR Team \nSponsored by: WSR Laboratory - SEEE - HUST";
            lblDesigner.Dock = DockStyle.Fill;
            lblDesigner.Font = new Font("Segoe UI", 9, FontStyle.Bold | FontStyle.Italic);
            lblDesigner.ForeColor = Color.DarkGreen;

            this.Load += SerialConfigUpdated;
            this.WindowState = FormWindowState.Maximized;
            this.Shown += MainFormFirstShown;
            this.AutoScaleMode = AutoScaleMode.Dpi;
            this.Icon = Properties.Resources.BalanceCarIcon;
            radAllValuesView.Checked = true;
            radXAngle.Checked = true;
            grpGraphChoice.Enabled = true;
        }
        private void MainDashboard_Load(object sender, EventArgs e)
        {
            // Khoi tao mot bo loc LPF ban dau
            filterDisplayData.Init(cutoffFrq, dataRecvSampTime);


            //originalFormSize = this.ClientSize;
            //foreach (Control ctrl in this.Controls)
            //{
            //    originalBounds[ctrl] = ctrl.Bounds;
            //}
            this.Resize += MainFormResize;
        }
        private void MainFormFirstShown(object sender, EventArgs e)
        {
            originalFormSize = this.ClientSize;
            originalBounds.Clear();
            AdjustPanelsLayout();
            GetAllOriginalControlBounds(this);
        }
        private void MainFormResize(object sender, EventArgs e)
        {
            originalFormSize = this.ClientSize;
            foreach (Control ctrl in this.Controls)
            {
                originalBounds[ctrl] = ctrl.Bounds;
            }
            AdjustPanelsLayout();
            AllControlsResize();
            //AllControlsResizeUpdated(this, originalFormSize.Width, originalFormSize.Height); 
        }
        private void GetAllOriginalControlBounds(Control parent)
        {
            originalParentControlSize = parent.Size;
            foreach (Control ctrl in parent.Controls)
            {
                originalBounds[ctrl] = ctrl.Bounds;
                if (ctrl.HasChildren)
                {
                    GetAllOriginalControlBounds(ctrl);
                }
            }
        }

        private void AdjustPanelsLayout()
        {
            // Phương thức thay đổi kích cỡ các panel khi form thay đổi kích thước
            pnData.Width = (int)(this.ClientSize.Width * 0.2);
            pnData.Height = (int)(this.ClientSize.Height * 0.94);
            pnDesigner.Width = (int)(this.ClientSize.Width * 0.2);
            pnDesigner.Height = (int)(this.ClientSize.Height * 0.06);
            pnControl.Width = (int)(this.ClientSize.Width * 0.72);
            pnControl.Height = (int)(this.ClientSize.Height * 0.1);
            pnGraph.Width = (int)(this.ClientSize.Width * 0.72);
            pnGraph.Height = (int)(this.ClientSize.Height * 0.9);
            pnGraphChoice.Width = (int)(this.ClientSize.Width * 0.08);
            pnGraphChoice.Height = (int)(this.ClientSize.Height * 1);

            pnData.Location = new Point(0, 0);
            pnDesigner.Location = new Point(0, pnData.Height);
            pnControl.Location = new Point(pnData.Width, 0);
            pnGraph.Location = new Point(pnData.Width, pnControl.Height);
            pnGraphChoice.Location = new Point(pnData.Width + pnControl.Width, 0);
        }
        private void AllControlsResizeUpdated(Control parent, int originalParentWidth, int originalParentHeight)
        {
            float xRatio = (float)parent.Width / (float)originalParentWidth;
            float yRatio = (float)parent.Height / (float)originalParentHeight;
            foreach (Control ctrl in parent.Controls)
            {
                Rectangle original = originalBounds[ctrl];
                int originalWidth = original.Width;
                int originalHeight = original.Height;
                if (ctrl is Panel)
                    continue;
                else
                {
                    ctrl.Left = (int)(original.Left * xRatio);
                    ctrl.Top = (int)(original.Top * yRatio);
                    ctrl.Width = (int)(original.Width * xRatio);
                    ctrl.Height = (int)(original.Height * yRatio);
                }
                if (ctrl is ZedGraphControl zgc)
                {
                    zgc.AxisChange();
                    zgc.Invalidate();
                }
                if (ctrl.HasChildren)
                {
                    AllControlsResizeUpdated(ctrl, originalWidth, originalHeight);
                }
            }
        }
        private void AllControlsResize()
        {
            float xRatio = (float)this.ClientSize.Width / originalFormSize.Width;
            float yRatio = (float)this.ClientSize.Height / originalFormSize.Height;

            foreach (Control ctrl in this.Controls)
            {
                if (ctrl is Panel)
                {
                    continue;
                }
                else
                {
                    Rectangle original = originalBounds[ctrl];
                    ctrl.Left = (int)(original.Left * xRatio);
                    ctrl.Top = (int)(original.Top * yRatio);
                    ctrl.Width = (int)(original.Width * xRatio);
                    ctrl.Height = (int)(original.Height * yRatio);
                }
                if (ctrl is ZedGraphControl zgc)
                {
                    zgc.AxisChange();
                    zgc.Invalidate();
                }
            }
        }
        private void FormatAllControls(Control parent)
        {
            // Định dạng toàn bộ các control trong giao diện
            foreach (Control ctrl in parent.Controls)
            {
                if (ctrl is System.Windows.Forms.Label lbl)
                {
                    lbl.AutoSize = true;
                    lbl.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    lbl.ForeColor = Color.DarkBlue;
                }
                else if (ctrl is TextBox txt)
                {
                    //txt.AutoSize = true;
                    txt.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    txt.ForeColor = Color.Black;
                }
                else if (ctrl is ComboBox cmb)
                {
                    //cmb.AutoSize = true;
                    cmb.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    cmb.ForeColor = Color.Black;
                }
                else if (ctrl is RadioButton rad)
                {
                    //rad.AutoSize = true;
                    rad.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    rad.ForeColor = Color.Purple;
                }
                else if (ctrl is GroupBox grp)
                {
                    //grp.AutoSize = true;
                    grp.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    grp.ForeColor = Color.BlueViolet;
                }
                else if (ctrl is Button btn)
                {
                    btn.Font = new Font("Segoe UI", 10, FontStyle.Bold);
                    btn.ForeColor = Color.Brown;
                    btn.BackColor = Color.OldLace;
                }
                if (ctrl.HasChildren)
                {
                    FormatAllControls(ctrl);
                }
            }
        }
        private void SerialConfigUpdated(object sender, EventArgs e)
        {
            // Thiết lập giá trị cổng COM và BaudRate đã lưu (nếu có)
            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastSerialPort))
                cmbSerialPort.SelectedItem = Properties.Settings.Default.LastSerialPort;

            if (!string.IsNullOrEmpty(Properties.Settings.Default.LastBaudRate))
                cmbBaudRate.SelectedItem = Properties.Settings.Default.LastBaudRate;
        }
        private void cmbSerialPort_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Lưu giá trị mặc định mới cho cổng COM
            Properties.Settings.Default.LastSerialPort = cmbSerialPort.SelectedItem.ToString();
            Properties.Settings.Default.Save();
        }
        private void cmbBaudRate_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Lưu giá trị mặc định mới cho BaudRate
            Properties.Settings.Default.LastBaudRate = cmbBaudRate.SelectedItem.ToString();
            Properties.Settings.Default.Save();
        }
        private void CreateSerialPort()
        {
            // Tạo cổng kết nối serial
            serCOM = new SerialPort() { BaudRate = 115200, PortName = "COM5", DataBits = 8, Parity = Parity.None, StopBits = StopBits.One };
            serCOM.DataReceived += SerCOM_DataReceived;
        }
        private void CreatePanels()
        {
            // Tạo ra các panel phân chia giao diện chính
            pnData.BorderStyle = BorderStyle.FixedSingle;
            pnDesigner.BorderStyle = BorderStyle.FixedSingle;
            pnControl.BorderStyle = BorderStyle.FixedSingle;
            pnGraph.BorderStyle = BorderStyle.FixedSingle;
            pnGraphChoice.BorderStyle = BorderStyle.FixedSingle;

            pnData.BackColor = Color.AliceBlue;
            pnDesigner.BackColor = Color.AliceBlue;
            pnControl.BackColor = Color.AliceBlue;
            pnGraph.BackColor = Color.OldLace;
            pnGraphChoice.BackColor = Color.AliceBlue;

            this.Controls.Add(pnData);
            this.Controls.Add(pnDesigner);
            this.Controls.Add(pnControl);
            this.Controls.Add(pnGraph);
            this.Controls.Add(pnGraphChoice);
        }
        private void CreateZedGraphGroups()
        {
            // Them cac doi tuong Zedgraph vao cac panel
            pnGraph.Controls.Add(zgc1); pnGraph.Controls.Add(zgc2); pnGraph.Controls.Add(zgc3); pnGraph.Controls.Add(zgc4);
            pnGraph.Controls.Add(zgc5); pnGraph.Controls.Add(zgc6); pnGraph.Controls.Add(zgc7); pnGraph.Controls.Add(zgc8);

            // Tao ra cac bieu do trong doi tuong Zedgraph
            mainDisplayPane = zgc1.GraphPane; xAnglePane = zgc2.GraphPane;
            zAnglePane = zgc3.GraphPane; xAngularVelocityPane = zgc4.GraphPane;
            zAngularVelocityPane = zgc5.GraphPane; xCoordinatePane = zgc6.GraphPane;
            leftTorquePane = zgc7.GraphPane; rightTorquePane = zgc8.GraphPane;

            // Cac thong tin lien quan den do thi
            mainDisplayPane.YAxis.Title.Text = "Giá trị"; mainDisplayPane.XAxis.Title.Text = "Thời gian (s)";

            xAnglePane.Title.Text = "Góc x";
            xAnglePane.YAxis.Title.Text = "Giá trị"; xAnglePane.XAxis.Title.Text = "Thời gian (s)";

            zAnglePane.Title.Text = "Góc z";
            zAnglePane.YAxis.Title.Text = "Giá trị"; zAnglePane.XAxis.Title.Text = "Thời gian (s)";

            xAngularVelocityPane.Title.Text = "Vận tốc góc x";
            xAngularVelocityPane.YAxis.Title.Text = "Giá trị"; xAngularVelocityPane.XAxis.Title.Text = "Thời gian (s)";

            zAngularVelocityPane.Title.Text = "Vận tốc góc z";
            zAngularVelocityPane.YAxis.Title.Text = "Giá trị"; zAngularVelocityPane.XAxis.Title.Text = "Thời gian (s)";

            xCoordinatePane.Title.Text = "Toạ độ x";
            xCoordinatePane.YAxis.Title.Text = "Giá trị"; xCoordinatePane.XAxis.Title.Text = "Thời gian (s)";

            leftTorquePane.Title.Text = "Momen bánh trái";
            leftTorquePane.YAxis.Title.Text = "Giá trị"; leftTorquePane.XAxis.Title.Text = "Thời gian (s)";

            rightTorquePane.Title.Text = "Momen bánh phải";
            rightTorquePane.YAxis.Title.Text = "Giá trị"; rightTorquePane.XAxis.Title.Text = "Thời gian (s)";

            // Dinh nghia cac duong Line de ve do thi
            xAngleLine = xAnglePane.AddCurve("Góc x", xAnglePoints, Color.Red, SymbolType.None);
            zAngleLine = zAnglePane.AddCurve("Góc z", zAnglePoints, Color.Blue, SymbolType.None);
            xAngularVelocityLine = xAngularVelocityPane.AddCurve("Vận tốc góc x", xAngularVelocityPoints, Color.Green, SymbolType.None);
            zAngularVelocityLine = zAngularVelocityPane.AddCurve("Vận tốc góc z", zAngularVelocityPoints, Color.Gray, SymbolType.None);
            xCoordinateLine = xCoordinatePane.AddCurve("Toạ độ x", xCoordinatePoints, Color.Purple, SymbolType.None);
            leftTorqueLine = leftTorquePane.AddCurve("Momen bánh trái", leftTorquePoints, Color.Orange, SymbolType.None);
            rightTorqueLine = rightTorquePane.AddCurve("Momen bánh phải", rightTorquePoints, Color.Violet, SymbolType.None);

            // Phan chia cac vach chia tren 2 truc cua do thi
            mainDisplayPane.XAxis.Scale.Min = 0; mainDisplayPane.XAxis.Scale.Max = 10;
            mainDisplayPane.XAxis.Scale.MinorStep = 0.1; // don vi chia nho nhat 0.1s
            mainDisplayPane.XAxis.Scale.MajorStep = 1; // don vi chia lon nhat 1s
            mainDisplayPane.XAxis.MajorGrid.IsVisible = true; mainDisplayPane.YAxis.MajorGrid.IsVisible = true;

            xAnglePane.XAxis.Scale.Min = 0; xAnglePane.XAxis.Scale.Max = 10;
            xAnglePane.XAxis.Scale.MinorStep = 0.1; // don vi chia nho nhat 0.1s
            xAnglePane.XAxis.Scale.MajorStep = 1; // don vi chia lon nhat 1s
            xAnglePane.XAxis.MajorGrid.IsVisible = true; xAnglePane.YAxis.MajorGrid.IsVisible = true;

            zAnglePane.XAxis.Scale.Min = 0; zAnglePane.XAxis.Scale.Max = 10;
            zAnglePane.XAxis.Scale.MinorStep = 0.1; // don vi chia nho nhat 0.1s
            zAnglePane.XAxis.Scale.MajorStep = 1; // don vi chia lon nhat 1s
            zAnglePane.XAxis.MajorGrid.IsVisible = true; zAnglePane.YAxis.MajorGrid.IsVisible = true;

            xAngularVelocityPane.XAxis.Scale.Min = 0; xAngularVelocityPane.XAxis.Scale.Max = 10;
            xAngularVelocityPane.XAxis.Scale.MinorStep = 0.1; // don vi chia nho nhat 0.1s
            xAngularVelocityPane.XAxis.Scale.MajorStep = 1; // don vi chia lon nhat 1s
            xAngularVelocityPane.XAxis.MajorGrid.IsVisible = true; xAngularVelocityPane.YAxis.MajorGrid.IsVisible = true;

            zAngularVelocityPane.XAxis.Scale.Min = 0; zAngularVelocityPane.XAxis.Scale.Max = 10;
            zAngularVelocityPane.XAxis.Scale.MinorStep = 0.1; // don vi chia nho nhat 0.1s
            zAngularVelocityPane.XAxis.Scale.MajorStep = 1; // don vi chia lon nhat 1s
            zAngularVelocityPane.XAxis.MajorGrid.IsVisible = true; zAngularVelocityPane.YAxis.MajorGrid.IsVisible = true;

            xCoordinatePane.XAxis.Scale.Min = 0; xCoordinatePane.XAxis.Scale.Max = 10;
            xCoordinatePane.XAxis.Scale.MinorStep = 0.1; // don vi chia nho nhat 0.1s
            xCoordinatePane.XAxis.Scale.MajorStep = 1; // don vi chia lon nhat 1s
            xCoordinatePane.XAxis.MajorGrid.IsVisible = true; xCoordinatePane.YAxis.MajorGrid.IsVisible = true;

            leftTorquePane.XAxis.Scale.Min = 0; leftTorquePane.XAxis.Scale.Max = 10;
            leftTorquePane.XAxis.Scale.MinorStep = 0.1; // don vi chia nho nhat 0.1s
            leftTorquePane.XAxis.Scale.MajorStep = 1; // don vi chia lon nhat 1s
            leftTorquePane.XAxis.MajorGrid.IsVisible = true; leftTorquePane.YAxis.MajorGrid.IsVisible = true;

            rightTorquePane.XAxis.Scale.Min = 0; rightTorquePane.XAxis.Scale.Max = 10;
            rightTorquePane.XAxis.Scale.MinorStep = 0.1; // don vi chia nho nhat 0.1s
            rightTorquePane.XAxis.Scale.MajorStep = 1; // don vi chia lon nhat 1s
            rightTorquePane.XAxis.MajorGrid.IsVisible = true; rightTorquePane.YAxis.MajorGrid.IsVisible = true;
        }
        private void CreateRadioButtonGroups()
        {
            // Tao bang chon do thi hien thi rieng le              
            grpGraphChoice.Text = "Hiển thị đồ thị";
            grpGraphChoice.Location = new Point(0, 0); grpGraphChoice.Size = new Size(120, 801);
            //grpGraphChoice.AutoSize = true;
            grpGraphChoice.Controls.Add(radXAngle);
            grpGraphChoice.Controls.Add(radZAngle);
            grpGraphChoice.Controls.Add(radXAngularVelocity);
            grpGraphChoice.Controls.Add(radZAngularVelocity);
            grpGraphChoice.Controls.Add(radXCoordinate);
            grpGraphChoice.Controls.Add(radLeftTorque);
            grpGraphChoice.Controls.Add(radRightTorque);

            radXAngle.Text = "deg_x"; radXAngle.Location = new Point(10, 80);
            radZAngle.Text = "deg_z"; radZAngle.Location = new Point(10, 183);
            radXAngularVelocity.Text = "omega_x"; radXAngularVelocity.Location = new Point(10, 286);
            radZAngularVelocity.Text = "omega_z"; radZAngularVelocity.Location = new Point(10, 389);
            radXCoordinate.Text = "x"; radXCoordinate.Location = new Point(10, 492);
            radLeftTorque.Text = "T_left"; radLeftTorque.Location = new Point(10, 595);
            radRightTorque.Text = "T_right"; radRightTorque.Location = new Point(10, 698);

            pnGraphChoice.Controls.Add(grpGraphChoice);

            // Tao bang chon che do hien thi do thi
            grpGraphViewMode.Text = "Chế độ xem đồ thị";
            grpGraphViewMode.Location = new Point(grpUploadControls.Width + 20, 0);
            grpGraphViewMode.AutoSize = true;
            grpGraphViewMode.Controls.Add(radMeshView);
            grpGraphViewMode.Controls.Add(radAllValuesView);
            //grpGraphViewMode.Controls.Add(radVehicleView);

            radMeshView.Text = "Lưới"; radMeshView.Location = new Point(20, 30);
            radAllValuesView.Text = "Tập trung"; radAllValuesView.Location = new Point(130, 30);
            //radVehicleView.Text = "Trực quan"; radVehicleView.Location = new Point(240, 30);

            pnControl.Controls.Add(grpGraphViewMode);

            // Su kien thay doi che do hien thi do thi tren giao dien
            radMeshView.CheckedChanged += radGraphViewMode_CheckedChanged;
            radAllValuesView.CheckedChanged += radGraphViewMode_CheckedChanged;
            //radVehicleView.CheckedChanged += radGraphViewMode_CheckedChanged;

            radXAngle.CheckedChanged += radGraphChoice_CheckedChanged;
            radZAngle.CheckedChanged += radGraphChoice_CheckedChanged;
            radXAngularVelocity.CheckedChanged += radGraphChoice_CheckedChanged;
            radZAngularVelocity.CheckedChanged += radGraphChoice_CheckedChanged;
            radXCoordinate.CheckedChanged += radGraphChoice_CheckedChanged;
            radLeftTorque.CheckedChanged += radGraphChoice_CheckedChanged;
            radRightTorque.CheckedChanged += radGraphChoice_CheckedChanged;
        }
        private void CreateButtonGroups()
        {
            // Cac button control nhan du lieu len
            btnConnect = new Button() { Text = "Kết nối", Location = new Point(20, 20), Size = new Size(80, 50) };
            btnDisconnect = new Button() { Text = "Ngắt kết nối", Location = new Point(120, 20), Size = new Size(80, 50) };
            btnRun = new Button() { Text = "Chạy", Location = new Point(220, 20), Size = new Size(80, 50) };
            btnSaveData = new Button() { Text = "Lưu dữ liệu", Location = new Point(320, 20), Size = new Size(80, 50) };
            btnExit = new Button() { Text = "Thoát", Location = new Point(420, 20), Size = new Size(80, 50) };

            pnControl.Controls.Add(grpUploadControls); grpUploadControls.Text = "Control nhận dữ liệu";
            grpUploadControls.Location = new Point(0, 0);
            grpUploadControls.Size = new Size(520, 80);
            grpUploadControls.Controls.Add(btnConnect); grpUploadControls.Controls.Add(btnDisconnect);
            grpUploadControls.Controls.Add(btnRun); grpUploadControls.Controls.Add(btnSaveData); grpUploadControls.Controls.Add(btnExit);

            btnConnect.Click += btnConnect_Click;
            btnDisconnect.Click += btnDisconnect_Click;
            btnRun.Click += btnRun_Click;
            btnSaveData.Click += btnSaveData_Click;
            btnExit.Click += btnExit_Click;

            // Cac button control gui lenh dieu khien cho xe
            btnRobotRun = new Button() { Text = "Chạy xe", Location = new Point(10, 30), Size = new Size(80, 50) };
            btnRobotStop = new Button() { Text = "Dừng xe", Location = new Point(100, 30), Size = new Size(80, 50) };
            btnRobotSysConfig = new Button() { Text = "Thiết lập cấu hình xe", Location = new Point(190, 30), Size = new Size(100, 50) };

            pnData.Controls.Add(grpDownloadControls); grpDownloadControls.Text = "Control gửi lệnh điều khiển";
            grpDownloadControls.Location = new Point(0, 500);
            grpDownloadControls.Size = new Size(307, 80);
            grpDownloadControls.AutoSize = true;
            grpDownloadControls.Controls.Add(btnRobotRun);
            grpDownloadControls.Controls.Add(btnRobotStop);
            grpDownloadControls.Controls.Add(btnRobotSysConfig);

            btnRobotRun.Click += btnRobotRun_Click;
            btnRobotStop.Click += btnRobotStop_Click;
            btnRobotSysConfig.Click += btnRobotSysConfig_Click;

            // Button help + label help initialization
            lblHelp = new System.Windows.Forms.Label() { Text = "Hướng dẫn", Location = new Point(5, 640), AutoSize = true };
            pnData.Controls.Add(lblHelp);

            btnHelp = new Button() { Location = new Point(95, 640), Size = new Size(30, 30) };
            Image iconImage = Properties.Resources.QuestionMarkIcon2;
            Image resizedImage = new Bitmap(iconImage, new Size(btnHelp.Width - 10, btnHelp.Height - 10));
            btnHelp.Image = resizedImage;
            pnData.Controls.Add(btnHelp);
        }

        private void btnRobotSysConfig_Click(object sender, EventArgs e)
        {
            // Click nut de mo form cau hinh thong so cho xe
            if (frmRobotSysChild == null || frmRobotSysChild.IsDisposed)
            {
                frmRobotSysChild = new RobotSystemConfig(this); // truyền form chính vào form con
                frmRobotSysChild.StartPosition = FormStartPosition.CenterScreen;
                frmRobotSysChild.Show();
            }
            else
            {
                frmRobotSysChild.Focus(); // nếu đã mở thì đưa nó lên trước
            }
        }
        private void CreateLabelAndTextboxGroups()
        {
            grpDataView.Text = "Dữ liệu hiển thị";
            grpDataView.Location = new Point(0, 80); grpDataView.Size = new Size(307, 250);
            //grpDataView.AutoSize = true;
            pnData.Controls.Add(grpDataView);
            grpDataView.Controls.Add(txtXAngle); grpDataView.Controls.Add(lblXAngle);
            txtXAngle.ReadOnly = true;
            grpDataView.Controls.Add(txtZAngle); grpDataView.Controls.Add(lblZAngle);
            txtZAngle.ReadOnly = true;
            grpDataView.Controls.Add(txtXAngularVelocity); grpDataView.Controls.Add(lblXAngularVelocity);
            txtXAngularVelocity.ReadOnly = true;
            grpDataView.Controls.Add(txtZAngularVelocity); grpDataView.Controls.Add(lblZAngularVelocity);
            txtZAngularVelocity.ReadOnly = true;
            grpDataView.Controls.Add(txtXCoordinate); grpDataView.Controls.Add(lblXCoordinate);
            txtXCoordinate.ReadOnly = true;
            grpDataView.Controls.Add(txtLeftTorque); grpDataView.Controls.Add(lblLeftTorque);
            txtLeftTorque.ReadOnly = true;
            grpDataView.Controls.Add(txtRightTorque); grpDataView.Controls.Add(lblRightTorque);
            txtRightTorque.ReadOnly = true;

            grpCommunication.Text = "Kết nối truyền thông";
            grpCommunication.Location = new Point(0, 350); grpCommunication.Size = new Size(307, 130);
            //grpCommunication.AutoSize = true;
            pnData.Controls.Add(grpCommunication);
            grpCommunication.Controls.Add(txtRunTime); grpCommunication.Controls.Add(lblRunTime);

            pnDesigner.Controls.Add(lblDesigner);
            lblDesigner.AutoSize = true;
        }
        private void CreateMenuStripGroups()
        {
            mnuStrip.BackColor = Color.AliceBlue;
            mnuStrip.Location = new Point(0, 0);
            mnuStrip.AutoSize = true;
            mnuStrip.Items.Add(fileToolStrip);
            mnuStrip.Items.Add(saveToolStrip);
            mnuStrip.Items.Add(editToolStrip);
            mnuStrip.Items.Add(deleteToolStrip);
            mnuStrip.Items.Add(selectToolStrip);
            mnuStrip.Items.Add(historyToolStrip);
            fileToolStrip.DropDownItems.Add(newToolStrip);

            pnData.Controls.Add(mnuStrip);
        }
        private void CreateComboBoxGroups()
        {
            grpCommunication.Controls.Add(lblSerialPort); grpCommunication.Controls.Add(lblBaudRate);
            grpCommunication.Controls.Add(cmbSerialPort); grpCommunication.Controls.Add(cmbBaudRate);

            lblSerialPort.Location = new Point(30, 30);
            lblSerialPort.Text = "Cổng COM upload";
            lblSerialPort.AutoSize = true;

            cmbSerialPort.Location = new Point(200, 30);
            cmbSerialPort.Size = new Size(70, 20);
            cmbSerialPort.Items.AddRange(serialPortList);

            lblBaudRate.Location = new Point(30, 60);
            lblBaudRate.Text = "Baud rate upload";
            lblBaudRate.AutoSize = true;

            cmbBaudRate.Location = new Point(200, 60);
            cmbBaudRate.Size = new Size(70, 20);
            cmbBaudRate.Items.AddRange(baudRateList);

            cmbSerialPort.SelectedIndexChanged += cmbSerialPort_SelectedIndexChanged;
            cmbBaudRate.SelectedIndexChanged += cmbBaudRate_SelectedIndexChanged;
        }
        private void CreatePictureBox()
        {
            picVehicle.BorderStyle = BorderStyle.FixedSingle;
            picVehicle.Location = new Point(328, 136);
            picVehicle.Size = new Size(450, 450);
            picVehicle.SizeMode = PictureBoxSizeMode.StretchImage;
            pnGraph.Controls.Add(picVehicle);
            picVehicle.Image = Properties.Resources.BalanceCar5;
        }

        /**** Phuong thuc doc du lieu tu Serial Port COM ****/
        private void SerCOM_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            // ReceiveData(); // Phuong thuc doc du lieu tu Serial Port COM
            try
            {
                int bytesToRead = serCOM.BytesToRead;
                byte[] tempBuffer = new byte[bytesToRead];
                serCOM.Read(tempBuffer, 0, bytesToRead);
                lock (serialBuffer)
                {
                    serialBuffer.AddRange(tempBuffer);
                }

                // Xử lý mảng dữ liệu ESP32 Master gửi lên trong một luồng mới
                this.BeginInvoke(new MethodInvoker(ProcessSerialBuffer));
            }
            catch (Exception ex)
            {
                MessageBox.Show("Lỗi khi đọc dữ liệu: " + ex.Message);
            }
        }

        /**** Phương thức xử lý mảng dữ liệu ESP32 Master gửi lên qua cổng COM ****/
        private void ProcessSerialBuffer()
        {
            lock (serialBuffer)
            {
                // Điều kiện để luôn luôn thực hiện được việc kiểm tra mảng dữ liệu 
                while (serialBuffer.Count >= structSize)
                {
                    // Tìm byte bắt đầu UPLINK_START_BYTE (UPLINK_START_BYTE có thể nằm ở vị trí bất kì trong mảng)
                    int startIndex = serialBuffer.IndexOf(UPLINK_START_BYTE);

                    // Nếu không có start byte
                    if (startIndex == -1)
                    {
                        serialBuffer.Clear(); // Không có start byte, frame dữ liệu rác và loại bỏ
                        return; // Thoát khỏi phương thức
                    }

                    // Nếu frame đọc được có UPLINK_START_BYTE, nhưng số lượng bytes
                    // tính từ vị trí của UPLINK_START_BYTE chưa đủ bằng kích thức của Struct dữ liệu
                    if (serialBuffer.Count < startIndex + structSize)
                    {
                        return; // Thoát khỏi phương thức. Chờ thêm dữ liệu đọc lên từ cổng COM, rồi mới xử lý tiếp
                    }

                    // Nếu frame đã có UPLINK_START_BYTE, số bytes đủ bằng với số bytes của Struct dữ liệu,
                    // -> kiểm tra byte kết thúc
                    if (serialBuffer[startIndex + structSize - 1] == END_BYTE)
                    {
                        // Nếu byte kết thúc là END_BYTE, tách đủ số bytes và lưu dữ liệu vào gói dữ liệu hợp lệ
                        byte[] validPacket = serialBuffer.Skip(startIndex).Take(structSize).ToArray();
                        serialBuffer.RemoveRange(0, startIndex + structSize);

                        ProcessValidPacket(validPacket); // Phương thức xử lý gói dữ liệu hợp lệ
                    }
                    else
                    {
                        // Nếu byte kết thúc không phải là END_BYTE
                        serialBuffer.RemoveAt(0); // Bỏ byte đầu để tránh kẹt vòng lặp
                    }
                }
            }
        }

        /**** Hàm tính mã Modbus CRC16 ****/
        private ushort Modbus_CRC16(byte[] data, int length)
        {
            ushort crc = 0xFFFF;
            for (int pos = 0; pos < length; pos++)
            {
                crc ^= (ushort)data[pos];
                for (int i = 0; i < 8; i++)
                {
                    if ((crc & 0x0001) != 0)
                    {
                        crc >>= 1;
                        crc ^= 0xA001;
                    }
                    else
                    {
                        crc >>= 1;
                    }
                }
            }
            return crc;
        }

        /**** Xử lý gói dữ liệu hợp lệ và cập nhật lên đồ thị ****/
        private void ProcessValidPacket(byte[] recvBuffer)
        {
            data = ByteArrayToStructure<DataReceive>(recvBuffer);

            // Kiểm tra Checksum (Tính CRC 15 bytes đầu tiên: từ 0 đến structSize - 4)
            ushort computedCRC = Modbus_CRC16(recvBuffer, structSize - 3);
            if (computedCRC != (ushort)data.checksumCRC16)
            {
                return; // Checksum sai -> loại bỏ gói tin
            }

            double time;
            bool isGraphRunTimeEntered = double.TryParse(txtRunTime.Text, out double result);

            if (isGraphRunTimeEntered)
            {
                time = ((Environment.TickCount & int.MaxValue) - tickStart) / 1000.0;
                if (time <= result)
                {
                    // Cập nhật đồ thị
                    xAnglePoints.Add(time, data.xAngle / 100.0);
                    zAnglePoints.Add(time, data.zAngle / 100.0);
                    xAngularVelocityPoints.Add(time, data.xAngularVelocity / 100.0);
                    zAngularVelocityPoints.Add(time, data.zAngularVelocity / 100.0);
                    xCoordinatePoints.Add(time, data.xCoordinate / 50.0);
                    leftTorquePoints.Add(time, data.leftTorque / 100.0);
                    rightTorquePoints.Add(time, data.rightTorque / 100.0);

                    UpdateGraph(time);
                    //UpdateTextBox(data);
                    //UpdateTextBoxWithLPF(data);
                }
                else
                {
                    tmrUpdateData.Stop();
                    return;
                }
            }
            else return;
        }
        /**** Cập nhật đồ thị ****/
        private void UpdateGraph(double time)
        {
            // Khởi tạo 1 delegate với hàm Lambda cập nhật đồ thị
            Action<ZedGraphControl> updateXScale = zgc =>
            {
                Scale xscale = zgc.GraphPane.XAxis.Scale;
                if (time > xscale.Max - xscale.MajorStep)
                {
                    xscale.Min = time - 10;
                    xscale.Max = time;
                }
                zgc.AxisChange();
                zgc.Invalidate();
            };
            updateXScale(zgc1);
            updateXScale(zgc2);
            updateXScale(zgc3);
            updateXScale(zgc4);
            updateXScale(zgc5);
            updateXScale(zgc6);
            updateXScale(zgc7);
            updateXScale(zgc8);
        }

        /**** Khởi tạo timer cập nhật ô hiển thị dữ liệu ****/
        private void TimerUpdateDataSetup()
        {
            tmrUpdateData.Interval = 200;
            tmrUpdateData.Tick += TimerUpdateData_Tick;
        }

        /**** Cập nhật ô hiển thị dữ liệu ****/
        private void TimerUpdateData_Tick(object sender, EventArgs e)
        {
            txtXAngle.Text = (data.xAngle / 100.0).ToString("F2");
            txtZAngle.Text = (data.zAngle / 100.0).ToString("F2");
            txtXAngularVelocity.Text = (data.xAngularVelocity / 100.0).ToString("F2");
            txtZAngularVelocity.Text = (data.zAngularVelocity / 100.0).ToString("F2");
            txtXCoordinate.Text = (data.xCoordinate / 50.0).ToString("F2");
            txtLeftTorque.Text = (data.leftTorque / 100.0).ToString("F2");
            txtRightTorque.Text = (data.rightTorque / 100.0).ToString("F2");
        }
        private void UpdateTextBox(DataReceive data)
        {
            txtXAngle.Text = (data.xAngle / 100.0).ToString();
            txtZAngle.Text = (data.zAngle / 100.0).ToString();
            txtXAngularVelocity.Text = (data.xAngularVelocity / 100.0).ToString();
            txtZAngularVelocity.Text = (data.zAngularVelocity / 100.0).ToString();
            txtXCoordinate.Text = (data.xCoordinate / 50.0).ToString();
            txtLeftTorque.Text = (data.leftTorque / 100.0).ToString();
            txtRightTorque.Text = (data.rightTorque / 100.0).ToString();
        }
        private void UpdateTextBoxWithLPF(DataReceive data)
        {
            txtXAngle.Text = filterDisplayData.Update(data.xAngle / 100.0).ToString("F2");
            txtZAngle.Text = filterDisplayData.Update(data.zAngle / 100.0).ToString("F2");
            txtXAngularVelocity.Text = filterDisplayData.Update(data.xAngularVelocity / 100.0).ToString("F2");
            txtZAngularVelocity.Text = filterDisplayData.Update(data.zAngularVelocity / 100.0).ToString("F2");
            txtXCoordinate.Text = filterDisplayData.Update(data.xCoordinate / 50.0).ToString("F2");
            txtLeftTorque.Text = filterDisplayData.Update(data.leftTorque / 100.0).ToString("F2");
            txtRightTorque.Text = filterDisplayData.Update(data.rightTorque / 100.0).ToString("F2");
        }

        /**** Phương thức chuyển mảng dữ liệu byte[] thành struct ****/
        public static T ByteArrayToStructure<T>(byte[] bytes) where T : struct
        {
            GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            try
            {
                return (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
            }
            finally
            {
                handle.Free();
            }
        }
        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (!serCOM.IsOpen)
            {
                if (cmbSerialPort.SelectedItem != null && cmbBaudRate.SelectedItem != null)
                {
                    string selectedPort = cmbSerialPort.SelectedItem.ToString();
                    int baudRate = Convert.ToInt32(cmbBaudRate.SelectedItem.ToString());
                    serCOM.PortName = selectedPort;
                    serCOM.BaudRate = baudRate;
                    serCOM.DataBits = 8;
                    serCOM.Parity = Parity.None;
                    serCOM.StopBits = StopBits.One;

                    try
                    {
                        serCOM.Open();
                        //ResetGraph();
                        //tickStart = Environment.TickCount;
                        //tmrUpdateData.Start();
                        MessageBox.Show("Đã kết nối với cổng " + serCOM.PortName + " thành công!");
                    }
                    catch
                    {
                        MessageBox.Show("Lỗi kết nối cổng COM, vui lòng kiểm tra lại!");
                    }
                }
                else if (cmbSerialPort.SelectedItem != null && cmbBaudRate.SelectedItem == null)
                    MessageBox.Show("Xin hãy chọn baud rate.");
                else if (cmbSerialPort.SelectedItem == null && cmbBaudRate.SelectedItem != null)
                    MessageBox.Show("Xin hãy chọn cổng COM.");
                else if (cmbSerialPort.SelectedItem == null && cmbBaudRate.SelectedItem == null)
                    MessageBox.Show("Xin hãy chọn cổng COM và baud rate.");
            }
            else
            {
                MessageBox.Show("Đã kết nối rồi!");
            }
        }
        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            serCOM.Close();
            if (!serCOM.IsOpen)
            {
                MessageBox.Show("Đã ngắt kết nối");
            }
        }
        private void btnRun_Click(object sender, EventArgs e)
        {
            if (!serCOM.IsOpen)
            {
                MessageBox.Show("Kiểm tra lại kết nối cổng COM!");
            }
            else
            {
                if (double.TryParse(txtRunTime.Text, out double result))
                {
                    ResetGraph();
                    tickStart = Environment.TickCount;
                    tmrUpdateData.Start();
                }
                else MessageBox.Show("Vui lòng nhập thời gian chạy hợp lệ!");
            }
        }
        private void ResetGraph()
        {
            xAnglePoints.Clear();
            zAnglePoints.Clear();
            xAngularVelocityPoints.Clear();
            zAngularVelocityPoints.Clear();
            xCoordinatePoints.Clear();
            leftTorquePoints.Clear();
            rightTorquePoints.Clear();

            mainDisplayPane.XAxis.Scale.Min = 0; mainDisplayPane.XAxis.Scale.Max = 10;
            xAnglePane.XAxis.Scale.Min = 0; xAnglePane.XAxis.Scale.Max = 10;
            zAnglePane.XAxis.Scale.Min = 0; zAnglePane.XAxis.Scale.Max = 10;
            xAngularVelocityPane.XAxis.Scale.Min = 0; xAngularVelocityPane.XAxis.Scale.Max = 10;
            zAngularVelocityPane.XAxis.Scale.Min = 0; zAngularVelocityPane.XAxis.Scale.Max = 10;
            xCoordinatePane.XAxis.Scale.Min = 0; xCoordinatePane.XAxis.Scale.Max = 10;
            leftTorquePane.XAxis.Scale.Min = 0; leftTorquePane.XAxis.Scale.Max = 10;
            rightTorquePane.XAxis.Scale.Min = 0; rightTorquePane.XAxis.Scale.Max = 10;
        }

        public void btnSaveData_Click(object sender, EventArgs e)
        {
            zedGraphControlsList = new List<ZedGraphControl>();
            tmrUpdateData.Stop();
            saveFileDialog.Filter = "Microsoft Excel Worksheets (*.xlsx)|*.xlsx | All Files (*.*)|*.*";
            if (saveFileDialog.ShowDialog() == DialogResult.OK)
            {
                zedGraphControlsList.Add(zgc2);
                zedGraphControlsList.Add(zgc3);
                zedGraphControlsList.Add(zgc4);
                zedGraphControlsList.Add(zgc5);
                zedGraphControlsList.Add(zgc6);
                zedGraphControlsList.Add(zgc7);
                zedGraphControlsList.Add(zgc8);
                SaveDataToExcel(zedGraphControlsList);
            }
        }

        private void SaveDataToExcel(List<ZedGraphControl> zedGraphs)
        {
            byte zedGraphNums = (byte)zedGraphs.Count;
            Console.WriteLine(zedGraphNums);
            ExcelPackage.LicenseContext = OfficeOpenXml.LicenseContext.NonCommercial;
            using (ExcelPackage excel = new ExcelPackage())
            {
                var ws = excel.Workbook.Worksheets.Add("ZedGraph Data");
                ws.Cells[1, 1].Value = "Time";
                ws.Cells[1, 2].Value = "Góc x"; ws.Cells[1, 3].Value = "Góc z"; ws.Cells[1, 4].Value = "Tốc độ góc x"; ws.Cells[1, 5].Value = "Tốc độ góc z";
                ws.Cells[1, 6].Value = "Toạ độ x"; ws.Cells[1, 7].Value = "Momen bánh trái"; ws.Cells[1, 8].Value = "Momen bánh phải";

                ws.Cells[1, 1, 1, zedGraphNums + 1].Style.Font.Bold = true;
                ws.Cells[1, 1, 1, zedGraphNums + 1].Style.Font.Size = 10;
                ws.Cells[1, 1, 1, zedGraphNums + 1].Style.Fill.PatternType = ExcelFillStyle.Solid;
                ws.Cells[1, 1, 1, zedGraphNums + 1].Style.Fill.BackgroundColor.SetColor(Color.AliceBlue);
                ws.Cells[1, 1, 1, zedGraphNums + 1].Style.Font.Color.SetColor(Color.Black);
                ws.Cells[1, 1, 1, zedGraphNums + 1].AutoFitColumns();
                int col = 2, row = 2;
                foreach (ZedGraphControl zgc in zedGraphs)
                {
                    var curve = zgc.GraphPane.CurveList[0];
                    if (curve is LineItem line)
                    {
                        IPointList ipointList = line.Points;  // Sử dụng IPointList thay vì PointPairList

                        // Tạo ra PointPairList và lưu dữ liệu từ biến ipointList
                        PointPairList pointPairList = new PointPairList();
                        for (int i = 0; i < ipointList.Count; i++)
                        {
                            pointPairList.Add(ipointList[i]);
                        }
                        if (pointPairList != null && pointPairList.Count > 0) // Kiểm tra xem có dữ liệu không
                        {
                            foreach (PointPair point in pointPairList)
                            {
                                ws.Cells[row, 1].Value = point.X;
                                ws.Cells[row, col].Value = point.Y;
                                row++;
                            }
                        }
                        row = 2;
                        col++;
                    }
                    else
                    {
                        MessageBox.Show("Curve does not contain a valid PointPairList.");
                        return;
                    }
                }
                col = 2;

                string filePath = saveFileDialog.FileName;
                File.WriteAllBytes(filePath, excel.GetAsByteArray());
                MessageBox.Show("Data exported to " + filePath);
            }
        }
        private ushort Modbus_CRC16(byte[] data, ushort frameLength, ushort pos_start, ushort pos_end)
        {
            ushort crc = 0xFFFF;
            for (ushort pos = pos_start; pos <= pos_end; pos++)
            {
                crc ^= (ushort)data[pos];
                for (byte i = 0; i < 8; i++)
                {
                    if ((crc & 0x0001) == 0x0001)
                    {
                        crc >>= 1;
                        crc ^= 0xA001;
                    }
                    else
                    {
                        crc >>= 1;
                    }
                }
            }
            return crc;
        }
        private void SendShortFrame(byte downlink_startByte, byte endByte, int DataType, int Index, ushort dataSend)
        {
            // Frame 9 BTYES {0: start/ 1: dữ liệu loại ()/ 2: loại frame/ 3: thứ tự trong loại/ 4-5: 2 byte dữ liệu/ 6-7: checksum/ 8: end}
            byte[] transBuff = new byte[9];
            transBuff[0] = downlink_startByte; transBuff[8] = endByte;
            transBuff[1] = (byte)DataType; transBuff[2] = 0x00;
            transBuff[3] = (byte)Index;
            transBuff[4] = (byte)(dataSend); transBuff[5] = (byte)(dataSend >> 8);
            ushort crc16 = Modbus_CRC16(transBuff, 9, 0, 5);
            transBuff[6] = (byte)(crc16 & 0xFF);        // Byte thấp của CRC16
            transBuff[7] = (byte)((crc16 >> 8) & 0xFF); // Byte cao của CRC16
            serCOM.Write(transBuff, 0, 9);
        }

        private void SendLongFrame(byte downlink_startByte, byte endByte, int DataType, ushort data_a, ushort data_beta, ushort data_k)
        {
            // Frame 12 BTYES {0: start/ 1: dữ liệu loại ()/ 2: loại frame/ 3-4: 2 byte data_a/ 5-6: 2 byte data_beta/ 7-8: 2 byte data_k/ 9-10: checksum/ 11: end}
            byte[] transBuff = new byte[12];
            transBuff[0] = downlink_startByte; transBuff[11] = endByte;
            transBuff[1] = (byte)DataType; transBuff[2] = 0x01; // 0x01 là frame dài

            transBuff[3] = (byte)(data_a); transBuff[4] = (byte)(data_a >> 8);
            transBuff[5] = (byte)(data_beta); transBuff[6] = (byte)(data_beta >> 8);
            transBuff[7] = (byte)(data_k); transBuff[8] = (byte)(data_k >> 8);

            // Crc16 được tính từ byte 0 đến byte 8 (tổng cộng 9 bytes)
            ushort crc16 = Modbus_CRC16(transBuff, 12, 0, 8);
            transBuff[9] = (byte)(crc16 & 0xFF);         // Byte thấp của CRC16
            transBuff[10] = (byte)((crc16 >> 8) & 0xFF); // Byte cao của CRC16
            serCOM.Write(transBuff, 0, 12);
        }

        private async void btnRobotRun_Click(object sender, EventArgs e)
        {
            if (serCOM.IsOpen)
            {
                // Frame 9 BTYES {0: start/ 1: dữ liệu loại ()/ 2: loại frame/ 3: thứ tự trong loại/ 4-5: 2 byte dữ liệu/ 6-7: checksum/ 8: end}
                // Với lệnh RUN: byte [1] dữ liệu loại 0; byte [3] thứ tự là 1; byte [4] và byte [5] là 0x00 và 0x00

                SendShortFrame(downlink_startByte, endByte, 0, 1, 0);
                await Task.Delay(50);
            }
            else MessageBox.Show("Cổng COM chưa được kết nối, xin hãy kiểm tra lại!");
        }
        private async void btnRobotStop_Click(object sender, EventArgs e)
        {
            if (serCOM.IsOpen)
            {
                // Frame 9 BTYES {0: start/ 1: dữ liệu loại ()/ 2: loại frame/ 3: thứ tự trong loại/ 4-5: 2 byte dữ liệu/ 6-7: checksum/ 8: end}
                // Với lệnh STOP: byte [1] dữ liệu loại 0; byte [3] thứ tự là 0; byte [4] và byte [5] là 0x00 và 0x00

                SendShortFrame(downlink_startByte, endByte, 0, 0, 0);
                await Task.Delay(50);
            }
            else MessageBox.Show("Cổng COM chưa được kết nối, xin hãy kiểm tra lại!");
        }

        static void PrintBinary(ushort value, byte bitCount)
        {
            for (int i = bitCount - 1; i >= 0; i--)
            {
                Console.Write(((value >> i) & 1) == 1 ? '1' : '0');
            }
            Console.WriteLine();
        }
        private void btnExit_Click(object sender, EventArgs e)
        {
            tmrUpdateData.Stop();
            serCOM.Close();
            Application.Exit();
        }
        private void radGraphViewMode_CheckedChanged(object radGraphView, EventArgs e)
        {
            if (((RadioButton)radGraphView).Checked)
            {
                if ((RadioButton)radGraphView == radMeshView)
                {
                    grpGraphChoice.Enabled = false; grpGraphChoice.Visible = true;
                    picVehicle.Visible = false;
                    zgc1.Visible = false;
                    zgc2.Visible = true; zgc3.Visible = true;
                    zgc4.Visible = true; zgc5.Visible = true;
                    zgc6.Visible = true; zgc7.Visible = true; zgc8.Visible = true;
                    zgc2.Location = new Point(0, 0); zgc2.Size = new Size(277, 361);
                    zgc3.Location = new Point(277, 0); zgc3.Size = new Size(277, 361);
                    zgc4.Location = new Point(554, 0); zgc4.Size = new Size(276, 361);
                    zgc5.Location = new Point(830, 0); zgc5.Size = new Size(276, 361);
                    zgc6.Location = new Point(0, 361); zgc6.Size = new Size(369, 361);
                    zgc7.Location = new Point(369, 361); zgc7.Size = new Size(369, 361);
                    zgc8.Location = new Point(738, 361); zgc8.Size = new Size(368, 361);
                    originalBounds[zgc1] = zgc1.Bounds;
                    originalBounds[zgc2] = zgc2.Bounds;
                    originalBounds[zgc3] = zgc3.Bounds;
                    originalBounds[zgc4] = zgc4.Bounds;
                    originalBounds[zgc5] = zgc5.Bounds;
                    originalBounds[zgc6] = zgc6.Bounds;
                    originalBounds[zgc7] = zgc7.Bounds;
                    originalBounds[zgc8] = zgc8.Bounds;
                }
                else if ((RadioButton)radGraphView == radAllValuesView)
                {
                    grpGraphChoice.Enabled = true; grpGraphChoice.Visible = true;
                    picVehicle.Visible = false;
                    zgc1.Visible = true;
                    zgc2.Visible = true; zgc3.Visible = true;
                    zgc4.Visible = true; zgc5.Visible = true;
                    zgc6.Visible = true; zgc7.Visible = true; zgc8.Visible = true;

                    zgc1.Location = new Point(0, 0); zgc1.Size = new Size(876, 721);
                    zgc2.Location = new Point(876, 0); zgc2.Size = new Size(230, 103);
                    zgc3.Location = new Point(876, 103); zgc3.Size = new Size(230, 103);
                    zgc4.Location = new Point(876, 206); zgc4.Size = new Size(230, 103);
                    zgc5.Location = new Point(876, 309); zgc5.Size = new Size(230, 103);
                    zgc6.Location = new Point(876, 412); zgc6.Size = new Size(230, 103);
                    zgc7.Location = new Point(876, 515); zgc7.Size = new Size(230, 103);
                    zgc8.Location = new Point(876, 618); zgc8.Size = new Size(230, 103);
                    originalBounds[zgc1] = zgc1.Bounds;
                    originalBounds[zgc2] = zgc2.Bounds;
                    originalBounds[zgc3] = zgc3.Bounds;
                    originalBounds[zgc4] = zgc4.Bounds;
                    originalBounds[zgc5] = zgc5.Bounds;
                    originalBounds[zgc6] = zgc6.Bounds;
                    originalBounds[zgc7] = zgc7.Bounds;
                    originalBounds[zgc8] = zgc8.Bounds;
                }
                //else if ((RadioButton)radGraphView == radVehicleView)
                //{
                //    grpGraphChoice.Enabled = false; //grpGraphChoice.Visible = false;
                //    zgc1.Visible = false;
                //    zgc2.Visible = false; zgc3.Visible = false;
                //    zgc4.Visible = false; zgc5.Visible = false;
                //    zgc6.Visible = false; zgc7.Visible = false; zgc8.Visible = false;
                //    picVehicle.Visible = true;
                //}
            }
        }
        private void radGraphChoice_CheckedChanged(object graphChoice, EventArgs e)
        {
            if (((RadioButton)graphChoice).Checked)
            {
                if ((RadioButton)graphChoice == radXAngle)
                {
                    UpdateMainGraph(1);
                }
                else if ((RadioButton)graphChoice == radZAngle)
                {
                    UpdateMainGraph(2);
                }
                else if ((RadioButton)graphChoice == radXAngularVelocity)
                {
                    UpdateMainGraph(3);
                }
                else if ((RadioButton)graphChoice == radZAngularVelocity)
                {
                    UpdateMainGraph(4);
                }
                else if ((RadioButton)graphChoice == radXCoordinate)
                {
                    UpdateMainGraph(5);
                }
                else if ((RadioButton)graphChoice == radLeftTorque)
                {
                    UpdateMainGraph(6);
                }
                else if ((RadioButton)graphChoice == radRightTorque)
                {
                    UpdateMainGraph(7);
                }
            }
        }
        private void UpdateMainGraph(int graphIndex)
        {
            //GraphPane mainDisplayPane = zgc1.GraphPane;
            mainDisplayPane.CurveList.Clear(); // Xóa dữ liệu cũ trên đồ thị chính

            // Dựa vào chỉ số đồ thị con để sao chép dữ liệu
            switch (graphIndex)
            {
                case 1:
                    mainDisplayPane.Title.Text = "Góc x";
                    mainDisplayPane.YAxis.Title.Text = "(độ)";
                    // Sao chép dữ liệu từ đồ thị con 1
                    mainDisplayPane.AddCurve("Góc x", xAnglePoints, Color.Red, SymbolType.None);
                    break;

                case 2:
                    mainDisplayPane.Title.Text = "Góc z";
                    mainDisplayPane.YAxis.Title.Text = "(độ)";
                    // Sao chép dữ liệu từ đồ thị con 2
                    mainDisplayPane.AddCurve("Góc z", zAnglePoints, Color.Blue, SymbolType.None);
                    break;

                case 3:
                    mainDisplayPane.Title.Text = "Tốc độ góc x";
                    mainDisplayPane.YAxis.Title.Text = "(độ/s)";
                    // Sao chép dữ liệu từ đồ thị con 3
                    mainDisplayPane.AddCurve("Tốc độ góc x", xAngularVelocityPoints, Color.Green, SymbolType.None);
                    break;

                case 4:
                    mainDisplayPane.Title.Text = "Tốc độ góc z";
                    mainDisplayPane.YAxis.Title.Text = "(độ/s)";
                    // Sao chép dữ liệu từ đồ thị con 4
                    mainDisplayPane.AddCurve("Tốc độ góc z", zAngularVelocityPoints, Color.Gray, SymbolType.None);
                    break;

                case 5:
                    mainDisplayPane.Title.Text = "Toạ độ x";
                    mainDisplayPane.YAxis.Title.Text = "(m)";
                    // Sao chép dữ liệu từ đồ thị con 5
                    mainDisplayPane.AddCurve("Toạ độ x", xCoordinatePoints, Color.Purple, SymbolType.None);
                    break;

                case 6:
                    mainDisplayPane.Title.Text = "Momen bánh trái";
                    mainDisplayPane.YAxis.Title.Text = "(N.m)";
                    // Sao chép dữ liệu từ đồ thị con 6
                    mainDisplayPane.AddCurve("Momen bánh trái", leftTorquePoints, Color.Orange, SymbolType.None);
                    break;

                case 7:
                    mainDisplayPane.Title.Text = "Momen bánh phải";
                    mainDisplayPane.YAxis.Title.Text = "(N.m)";
                    // Sao chép dữ liệu từ đồ thị con 7
                    mainDisplayPane.AddCurve("Momen bánh phải", rightTorquePoints, Color.Violet, SymbolType.None);
                    break;
            }
            // Vẽ lại đồ thị chính
            zgc1.AxisChange();
            zgc1.Invalidate();
        }
        /*************************************/
    }
}

