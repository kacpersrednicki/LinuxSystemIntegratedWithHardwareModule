`timescale 1ns/100ps

module gpioemu_tb;

	reg n_reset;
	reg [15:0] saddress;
	reg srd;
	reg swr;
	reg [31:0] sdata_in;
	reg [15:0] gpio_in;
	reg gpio_latch;
	reg clk = 1;
	wire [31:0] sdata_out;
	wire [15:0] gpio_out;
	wire [15:0] gpio_in_s_insp;


	initial begin
		$dumpfile("gpioemu.vcd");
		$dumpvars(0, gpioemu_tb);
	end

	initial begin
		forever begin
			# 1 clk = ~clk;
		end
	end

	initial begin
		// reset
		# 1 n_reset = 1;
		# 1 n_reset = 0;
		# 100;
		
		// zapis argumentu
		# 1 saddress = 16'h238;
		# 1 sdata_in = 12;
		# 1 swr = 1;
		# 1 swr = 0;
		# 1000;
		
		// zapis zly adres (podtrzymanie wartosci)
		# 1 saddress = 16'h123;
		# 1 sdata_in = 32'b11111111111111111111111111111111;
		# 1 swr = 1;
		# 1 swr = 0;
		# 500;
		
		// odczyt wyniku
		# 1 saddress = 16'h248;
		# 1 gpio_latch = 1;
	        # 1 gpio_latch = 0;
	        # 1 srd = 1;
		# 1 srd = 0;
		# 500;
		
		// odczyt zly adres (neutralnosc odczytu)
		# 1 saddress = 16'h123;
	        # 1 gpio_latch = 1;
		# 1 gpio_latch = 0;
		# 1 srd = 1;
		# 1 srd = 0;
		# 500;
		
		// zapis argumentu
		# 1 saddress = 16'h238;
		# 1 sdata_in = 274;
		# 1 swr = 1;
		# 1 swr = 0;
		# 100000;
		
		// odczyt statusu
		# 1 saddress = 16'h250;
		# 1 gpio_latch = 1;
		# 1 gpio_latch = 0;
		# 1 srd = 1;
		# 1 srd = 0;
		# 200000;
		
		// odczyt wyniku
		# 1 saddress = 16'h248;
		# 1 gpio_latch = 1;
		# 1 gpio_latch = 0;
		# 1 srd = 1;
		# 1 srd = 0;
		# 200000;
		
		// zapis argumentu
		# 1 saddress = 16'h238;
		# 1 sdata_in = 946;
		# 1 swr = 1;
		# 1 swr = 0;
		# 3500000;
		
		// odczyt wyniku
		# 1 saddress = 16'h248;
		# 1 gpio_latch = 1;
		# 1 gpio_latch = 0;
		# 1 srd = 1;
		# 1 srd = 0;
		# 500000;
		
		// reset
		# 1 n_reset = 1;
		# 1 n_reset = 0;
		# 100;
		
		# 500000 $finish;
	end

	gpioemu prime_calc(n_reset, saddress, srd, swr, sdata_in, sdata_out, gpio_in, gpio_latch, gpio_out, gpio_in_s_insp, clk);

endmodule

