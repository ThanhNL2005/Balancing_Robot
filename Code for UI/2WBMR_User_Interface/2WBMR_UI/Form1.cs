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
using System.IO;
using System.Net.Sockets;
using OfficeOpenXml;

namespace _2WBMR_UI
{
    public partial class Form1 : Form
    {
        TcpClient client;
        StreamReader reader;

        int tickStart = 0; // khai bao bien dung cho timer, chay cot thoi gian tinh bang ms

        ZedGraphControl zgc2 = new ZedGraphControl();
        ZedGraphControl zgc3 = new ZedGraphControl();
        ZedGraphControl zgc4 = new ZedGraphControl();
        ZedGraphControl zgc5 = new ZedGraphControl();
        ZedGraphControl zgc6 = new ZedGraphControl();
        ZedGraphControl zgc7 = new ZedGraphControl();

        
        public Form1()
        {
            InitializeComponent();
            CreateRadioButtonGroups();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // Chon mac dinh che do hien thi tap trung
            radTapTrung.Checked = true;
            // Goi su kien de hien thi do thi dung cach
            radTapTrung_CheckedChanged(null, null);

            // Khoi tao doi tuong
            this.Controls.Add(zgc2); this.Controls.Add(zgc3); this.Controls.Add(zgc4);
            this.Controls.Add(zgc5); this.Controls.Add(zgc6); this.Controls.Add(zgc7);
            
            // Khoi tao bieu do
            GraphPane myPane1 = zedGraphControl1.GraphPane; // Khai bao su dung graph loai Graph Pane
            GraphPane myPane2 = zgc2.GraphPane;
            GraphPane myPane3 = zgc3.GraphPane;
            GraphPane myPane4 = zgc4.GraphPane;
            GraphPane myPane5 = zgc5.GraphPane;
            GraphPane myPane6 = zgc6.GraphPane;
            GraphPane myPane7 = zgc7.GraphPane;

            // Cac thong tin lien quan den do thi
            myPane1.Title.Text = "Đồ thị dữ liệu thu thập";
            myPane1.YAxis.Title.Text = "Giá trị"; myPane1.XAxis.Title.Text = "Thời gian (s)";
            
            myPane2.Title.Text = "Tốc độ xe";
            myPane2.YAxis.Title.Text = "Giá trị (m/s)"; myPane2.XAxis.Title.Text = "Thời gian (s)";

            myPane3.Title.Text = "Góc nghiêng thân xe";
            myPane3.YAxis.Title.Text = "Giá trị (độ)"; myPane3.XAxis.Title.Text = "Thời gian (s)";

            myPane4.Title.Text = "Góc đánh lái (độ)";
            myPane4.YAxis.Title.Text = "Giá trị"; myPane4.XAxis.Title.Text = "Thời gian (s)";

            myPane5.Title.Text = "Momen bánh trái (Nm)";
            myPane5.YAxis.Title.Text = "Giá trị"; myPane5.XAxis.Title.Text = "Thời gian (s)";

            myPane6.Title.Text = "Momen bánh phải (Nm)";
            myPane6.YAxis.Title.Text = "Giá trị"; myPane6.XAxis.Title.Text = "Thời gian (s)";

            myPane7.Title.Text = "Biến thứ 6";
            myPane7.YAxis.Title.Text = "Giá trị"; myPane7.XAxis.Title.Text = "Thời gian (s)";

            // Dinh nghia list de ve do thi
            RollingPointPairList list1_1 = new RollingPointPairList(1200);
            RollingPointPairList list1_2 = new RollingPointPairList(1200);
            RollingPointPairList list1_3 = new RollingPointPairList(1200);
            RollingPointPairList list1_4 = new RollingPointPairList(1200);
            RollingPointPairList list1_5 = new RollingPointPairList(1200);
            RollingPointPairList list1_6 = new RollingPointPairList(1200);
            


            LineItem curve1_1 = myPane1.AddCurve("Tốc độ xe", list1_1, Color.Red, SymbolType.None);
            LineItem curve1_2 = myPane1.AddCurve("Góc nghiêng", list1_2, Color.Blue, SymbolType.None);
            LineItem curve1_3 = myPane1.AddCurve("Góc đánh lái", list1_3, Color.Green, SymbolType.None);
            LineItem curve1_4 = myPane1.AddCurve("Momen bánh trái", list1_4, Color.Purple, SymbolType.None);
            LineItem curve1_5 = myPane1.AddCurve("Momen bánh phải", list1_5, Color.Orange, SymbolType.None);
            LineItem curve1_6 = myPane1.AddCurve("Biến thứ 6", list1_6, Color.Gray, SymbolType.None);
            LineItem curve2 = myPane2.AddCurve("Tốc độ xe", list1_1, Color.Red, SymbolType.None);
            LineItem curve3 = myPane3.AddCurve("Góc nghiêng", list1_2, Color.Blue, SymbolType.None);
            LineItem curve4 = myPane4.AddCurve("Góc đánh lái", list1_3, Color.Green, SymbolType.None);
            LineItem curve5 = myPane5.AddCurve("Momen bánh trái", list1_4, Color.Purple, SymbolType.None);
            LineItem curve6 = myPane6.AddCurve("Momen bánh phải", list1_5, Color.Orange, SymbolType.None);
            LineItem curve7 = myPane7.AddCurve("Biến thứ 6", list1_6, Color.Gray, SymbolType.None);


            timer1.Interval = 50; // Khoang cach la 50ms cho moi lan cap nhat diem

            myPane1.XAxis.Scale.Min = 0; myPane1.XAxis.Scale.Max = 30;
            myPane1.XAxis.Scale.MinorStep = 1; // don vi chia nho nhat 1
            myPane1.XAxis.Scale.MajorStep = 5; // don vi chia lon nhat 5
            myPane1.XAxis.MajorGrid.IsVisible = true; myPane1.YAxis.MajorGrid.IsVisible = true;

            myPane2.XAxis.Scale.Min = 0; myPane2.XAxis.Scale.Max = 30;
            myPane2.XAxis.Scale.MinorStep = 1; // don vi chia nho nhat 1
            myPane2.XAxis.Scale.MajorStep = 5; // don vi chia lon nhat 5
            myPane2.XAxis.MajorGrid.IsVisible = true; myPane2.YAxis.MajorGrid.IsVisible = true;

            myPane3.XAxis.Scale.Min = 0; myPane3.XAxis.Scale.Max = 30;
            myPane3.XAxis.Scale.MinorStep = 1; // don vi chia nho nhat 1
            myPane3.XAxis.Scale.MajorStep = 5; // don vi chia lon nhat 5
            myPane3.XAxis.MajorGrid.IsVisible = true; myPane3.YAxis.MajorGrid.IsVisible = true;

            myPane4.XAxis.Scale.Min = 0; myPane4.XAxis.Scale.Max = 30;
            myPane4.XAxis.Scale.MinorStep = 1; // don vi chia nho nhat 1
            myPane4.XAxis.Scale.MajorStep = 5; // don vi chia lon nhat 5
            myPane4.XAxis.MajorGrid.IsVisible = true; myPane4.YAxis.MajorGrid.IsVisible = true;

            myPane5.XAxis.Scale.Min = 0; myPane5.XAxis.Scale.Max = 30;
            myPane5.XAxis.Scale.MinorStep = 1; // don vi chia nho nhat 1
            myPane5.XAxis.Scale.MajorStep = 5; // don vi chia lon nhat 5
            myPane5.XAxis.MajorGrid.IsVisible = true; myPane5.YAxis.MajorGrid.IsVisible = true;

            myPane6.XAxis.Scale.Min = 0; myPane6.XAxis.Scale.Max = 30;
            myPane6.XAxis.Scale.MinorStep = 1; // don vi chia nho nhat 1
            myPane6.XAxis.Scale.MajorStep = 5; // don vi chia lon nhat 5
            myPane6.XAxis.MajorGrid.IsVisible = true; myPane6.YAxis.MajorGrid.IsVisible = true;

            myPane7.XAxis.Scale.Min = 0; myPane7.XAxis.Scale.Max = 30;
            myPane7.XAxis.Scale.MinorStep = 1; // don vi chia nho nhat 1
            myPane7.XAxis.Scale.MajorStep = 5; // don vi chia lon nhat 5
            myPane7.XAxis.MajorGrid.IsVisible = true; myPane7.YAxis.MajorGrid.IsVisible = true;

            // zedGraphControl1.AxisChange(); // goi ham xac dinh co truc
            
            
            // khoi dong timer ve vi tri ban dau
            tickStart = Environment.TickCount;

        }
        private void CreateRadioButtonGroups()
        {
            // Tao bang chon do thi hien thi rieng le
            GroupBox grpGraphChoice = new GroupBox();
            grpGraphChoice.Text = "Hiển thị đồ thị";
            grpGraphChoice.Size = new System.Drawing.Size(50, 660);
            grpGraphChoice.Location = new System.Drawing.Point(1450, 100);
            RadioButton radVelocity = new RadioButton() {Text = "", Location= new System.Drawing.Point(1450, 155)};
            RadioButton radTiltAngle = new RadioButton() { Text = "", Location = new System.Drawing.Point(1450, 265) }; ;
            RadioButton radSteerAngle = new RadioButton() { Text = "", Location = new System.Drawing.Point(1450, 375) }; ;
            RadioButton radLeftTorque = new RadioButton() { Text = "", Location = new System.Drawing.Point(1450, 485) }; ;
            RadioButton radRightTorque = new RadioButton() { Text = "", Location = new System.Drawing.Point(1450, 595) }; ;
            RadioButton radSixthVar = new RadioButton() { Text = "", Location = new System.Drawing.Point(1450, 705) }; ;
            grpGraphChoice.Controls.Add(radVelocity);
            grpGraphChoice.Controls.Add(radTiltAngle);
            grpGraphChoice.Controls.Add(radSteerAngle);
            grpGraphChoice.Controls.Add(radLeftTorque);
            grpGraphChoice.Controls.Add(radRightTorque);
            grpGraphChoice.Controls.Add(radSixthVar);
            this.Controls.Add(grpGraphChoice);
            
        }
        public void draw(double sp1, double sp2, double sp3, double sp4, double sp5, double sp6)
        {
            // kiem tra viec khoi tao cac duong curve
            if (zedGraphControl1.GraphPane.CurveList.Count <= 0 || zgc2.GraphPane.CurveList.Count <=0 ||
                zgc3.GraphPane.CurveList.Count <= 0 || zgc4.GraphPane.CurveList.Count <= 0 ||
                zgc5.GraphPane.CurveList.Count <= 0 || zgc6.GraphPane.CurveList.Count <= 0 ||
                zgc7.GraphPane.CurveList.Count <= 0)
                return; 
            
            // Dua diem ve vach xuat phat
            LineItem curve1_1 = zedGraphControl1.GraphPane.CurveList[0] as LineItem;
            LineItem curve1_2 = zedGraphControl1.GraphPane.CurveList[1] as LineItem;
            LineItem curve1_3 = zedGraphControl1.GraphPane.CurveList[2] as LineItem;
            LineItem curve1_4 = zedGraphControl1.GraphPane.CurveList[3] as LineItem;
            LineItem curve1_5 = zedGraphControl1.GraphPane.CurveList[4] as LineItem;
            LineItem curve1_6 = zedGraphControl1.GraphPane.CurveList[5] as LineItem;
            LineItem curve2 = zgc2.GraphPane.CurveList[0] as LineItem;
            LineItem curve3 = zgc3.GraphPane.CurveList[0] as LineItem;
            LineItem curve4 = zgc4.GraphPane.CurveList[0] as LineItem;
            LineItem curve5 = zgc5.GraphPane.CurveList[0] as LineItem;
            LineItem curve6 = zgc6.GraphPane.CurveList[0] as LineItem;
            LineItem curve7 = zgc7.GraphPane.CurveList[0] as LineItem;

            //if (curve1_1 == null) return;
            //if (curve1_2 == null) return;
            //if (curve1_3 == null) return;
            //if (curve1_4 == null) return;
            //if (curve1_5 == null) return;
            //if (curve1_6 == null) return;
            if (curve1_1 == null||curve1_2 == null||curve1_3 == null||curve1_4 == null||curve1_5 == null||curve1_6 == null||
                curve2 == null|| curve3 == null || curve4 == null || curve5 == null || curve6 == null || curve7 == null) 
                return;

            // List chua cac diem. Get the pointpairlist

            
            IPointListEdit list1_1, list1_2, list1_3, list1_4, list1_5, list1_6,
                            list2, list3, list4, list5, list6, list7;
            list1_1 = curve1_1.Points as IPointListEdit; list1_2 = curve1_2.Points as IPointListEdit;
            list1_3 = curve1_3.Points as IPointListEdit; list1_4 = curve1_4.Points as IPointListEdit;
            list1_5 = curve1_5.Points as IPointListEdit; list1_6 = curve1_6.Points as IPointListEdit;
            list2 = curve2.Points as IPointListEdit; list3 = curve3.Points as IPointListEdit;
            list4 = curve4.Points as IPointListEdit; list5 = curve5.Points as IPointListEdit;
            list6 = curve6.Points as IPointListEdit; list7 = curve7.Points as IPointListEdit;

            //if (list1 == null) return;
            //if (list2 == null) return;
            //if (list3 == null) return;
            //if (list4 == null) return;
            //if (list5 == null) return;
            //if (list6 == null) return;

            // Timer duoc tinh bang ms
            double time = (Environment.TickCount - tickStart) / 1000.0;

            // Hien thi du lieu len do thi
            list1_1.Add(time, sp1); list1_2.Add(time, sp2); list1_3.Add(time, sp3);
            list1_4.Add(time, sp4); list1_5.Add(time, sp5); list1_6.Add(time, sp6);
            list2.Add(time, sp1); list3.Add(time, sp2); list4.Add(time, sp3);
            list5.Add(time, sp4); list6.Add(time, sp5); list7.Add(time, sp6);

            // Chuong trinh ve do thi
            Scale xscale1 = zedGraphControl1.GraphPane.XAxis.Scale;
            Scale xscale2 = zgc2.GraphPane.XAxis.Scale;
            Scale xscale3 = zgc3.GraphPane.XAxis.Scale;
            Scale xscale4 = zgc4.GraphPane.XAxis.Scale;
            Scale xscale5 = zgc5.GraphPane.XAxis.Scale;
            Scale xscale6 = zgc6.GraphPane.XAxis.Scale;
            Scale xscale7 = zgc7.GraphPane.XAxis.Scale;
            if (time > xscale1.Max - xscale1.MajorStep)
            {
                xscale1.Min = time - 30;  // Giữ 30 giây dữ liệu trên màn hình
                xscale1.Max = time;       // Dịch trục X sang phải
                xscale2.Min = time - 30; xscale2.Max = time;         
                xscale3.Min = time - 30; xscale3.Max = time;       
                xscale4.Min = time - 30; xscale4.Max = time;       
                xscale5.Min = time - 30; xscale5.Max = time;       
                xscale6.Min = time - 30; xscale6.Max = time;      
                xscale7.Min = time - 30; xscale7.Max = time;
            }
            zedGraphControl1.AxisChange();  zedGraphControl1.Invalidate();
            zgc2.AxisChange();              zgc2.Invalidate();
            zgc3.AxisChange();              zgc3.Invalidate();
            zgc4.AxisChange();              zgc4.Invalidate();
            zgc5.AxisChange();              zgc5.Invalidate();
            zgc6.AxisChange();              zgc6.Invalidate();
            zgc7.AxisChange();              zgc7.Invalidate();

        }
                
        private void btnConnect_Click(object sender, EventArgs e)
        {
            try
            {
                client = new TcpClient("192.168.1.100", 1234); // IP ESP32, cần tuỳ chỉnh
                reader = new StreamReader(client.GetStream());
                MessageBox.Show("Kết nối ESP32 thành công!");

                while (true)
                {
                    string data = reader.ReadLine();
                    Console.WriteLine("Nhận từ ESP32: " + data);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Lỗi kết nối với ESP32, xin vui lòng kiểm tra lại");
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            draw(5, 10, 15, 20, 25, 30);        
        }

        private void btnExit_Click(object sender, EventArgs e)
        {
            timer1.Enabled = false;
            DialogResult result = MessageBox.Show("Bạn có chắc chắn muốn thoát ứng dụng?", "Xác nhận", MessageBoxButtons.YesNo);
            if (result == DialogResult.Yes)
            {
                Application.Exit();
            } 
                
        }

        private void btnRun_Click(object sender, EventArgs e)
        {
            timer1.Enabled = true;
        }

        private void radTapTrung_CheckedChanged(object sender, EventArgs e)
        {
            if (radTapTrung.Checked)
            {
                zedGraphControl1.Visible = true;
                //zedGraphControl1.Location = new Point(400, 100);
                //zedGraphControl1.Size = new Size(900, 660);

                zgc2.Location = new Point(1200, 100);
                zgc2.Size = new Size(230, 110);

                zgc3.Location = new Point(1200, 210);
                zgc3.Size = new Size(230, 110);

                zgc4.Location = new Point(1200, 320);
                zgc4.Size = new Size(230, 110);

                zgc5.Location = new Point(1200, 430);
                zgc5.Size = new Size(230, 110);

                zgc6.Location = new Point(1200, 540);
                zgc6.Size = new Size(230, 110);

                zgc7.Location = new Point(1200, 650);
                zgc7.Size = new Size(230, 110);

                
            }
        }

        private void radLuoi_CheckedChanged(object sender, EventArgs e)
        {
            if (radLuoi.Checked)
            {
                zedGraphControl1.Visible = false;

                zgc2.Location = new Point(300, 100);
                zgc2.Size = new Size(380, 340);

                zgc3.Location = new Point(680, 100);
                zgc3.Size = new Size(380, 340);

                zgc4.Location = new Point(1060, 100);
                zgc4.Size = new Size(380, 340);

                zgc5.Location = new Point(300, 440);
                zgc5.Size = new Size(380, 340);

                zgc6.Location = new Point(680, 440);
                zgc6.Size = new Size(380, 340);

                zgc7.Location = new Point(1060, 440);
                zgc7.Size = new Size(380, 340);
            }
        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            zedGraphControl1.GraphPane.CurveList.Clear(); // Xóa toàn bộ đường đồ thị và giữ lại khung đồ thị
            zedGraphControl1.Invalidate(); // Cập nhật lại đường đồ thị
            zgc2.GraphPane.CurveList.Clear();
            zgc2.Invalidate();
            zgc3.GraphPane.CurveList.Clear();
            zgc3.Invalidate();
            zgc4.GraphPane.CurveList.Clear();
            zgc4.Invalidate();
            zgc5.GraphPane.CurveList.Clear();
            zgc5.Invalidate();
            zgc6.GraphPane.CurveList.Clear();
            zgc6.Invalidate();
            zgc7.GraphPane.CurveList.Clear();
            zgc7.Invalidate();
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            
        }

        private void txtVelocity_TextChanged(object sender, EventArgs e)
        {
            //IPointList velocityPoints = zgc2.GraphPane.CurveList[0].Points;
            //string velocityText = "";
            //foreach (PointPair point in velocityPoints)
            //{
            //    velocityText += $"{point.Y}";    
            //}
            //txtVelocity.Text = velocityText;
            
        }

        private void groupBox1_Enter(object sender, EventArgs e)
        {

        }
    }
}
