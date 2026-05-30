using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace _2WBMR_UI_ver2
{
    public class LowPassFilter
    {
        private double Ts;                // Sampling time (s)
        private ushort Fc;                // Cutoff frequency (Hz)
        private double a1, a2, b1, b2;    // Parameters of the filter
        private double alpha;
        private double uk, uk_1;
        private double yk, yk_1, yk_2;

        public void Init(ushort CutoffFrqHz, double samplingTime)
        {
            double temp1, temp2;

            Ts = samplingTime;
            Fc = CutoffFrqHz;
            alpha = 1.0 / (2.0 * (double)Math.PI * Ts * Fc);

            temp1 = 1 + alpha;
            temp2 = temp1 * temp1;
            a1 = (1 + 2 * alpha) / temp2;
            a2 = -2 * alpha / temp2;
            b1 = -2 * alpha / temp1;
            b2 = alpha * alpha / temp2;

            uk = uk_1 = 0;
            yk = yk_1 = yk_2 = 0;
        }

        public double Update(double input)
        {
            // Read LPF input
            uk = input;
            // Compute LPF output
            yk = a1 * uk + a2 * uk_1 - b1 * yk_1 - b2 * yk_2;
            // Save LPF past data
            yk_2 = yk_1;
            yk_1 = yk;
            uk_1 = uk;
            //Return LPF output
            return yk;
        }
    }
}
