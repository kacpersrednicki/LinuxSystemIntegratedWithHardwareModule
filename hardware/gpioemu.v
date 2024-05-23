/*verilator lint_off UNUSED*/
/*verilator lint_off WIDTH*/
/*verilator lint_off MULTIDRIVEN*/
/*verilator lint_off UNDRIVEN*/
/*verilator lint_off CASEINCOMPLETE*/

module gpioemu(n_reset, saddress[15:0], srd, swr, sdata_in[31:0], sdata_out[31:0],
		gpio_in[15:0], gpio_latch, gpio_out[15:0], gpio_in_s_insp[15:0], clk);
	
	input n_reset;
	input [15:0] saddress;
	input srd;
	input swr;
	input [31:0] sdata_in;
	output [31:0] sdata_out;
	input [15:0] gpio_in;
	input gpio_latch;
	output [15:0] gpio_out;
	output [31:0] gpio_in_s_insp;
	reg [15:0] gpio_in_s;
	reg [15:0] gpio_out_s;
	reg [31:0] sdata_out_s;
	input clk;

	// wykorzystywane rejestry
	reg [9:0] A; // argument
	reg [9:0] S; // status
	reg [31:0] W; // wynik
	reg [15:0] current_number;
	reg [15:0] potential_factor;
	reg start;
	reg [2:0] state_reg;

	// adresy
	localparam A_address = 16'h238;
	localparam S_address = 16'h250;
	localparam W_address = 16'h248;
	
	// stany automatu
	localparam [2:0]	idle = 3'h0,
				is_ready = 3'h1,
				check_if_factor = 3'h2,
				increment_status = 3'h3,
				final_state = 3'h4;
	
	// reset 
	always @(negedge n_reset) begin
		sdata_out_s <= 0; 
		gpio_out_s <= 0;
		gpio_in_s <= 0;  
		A <= 0;
		S <= 0;
		W <= 0;
	end
	
	// zatrzask
	always @(posedge gpio_latch) begin
		gpio_in_s <= gpio_in;
	end

	// odczyt
	always @(posedge srd) begin
		sdata_out_s <= 0;
		case (saddress)
			W_address: sdata_out_s <= W;
			S_address: sdata_out_s <= S;
			A_address: sdata_out_s <= A;
		endcase
	end
	
	// zapis
	always @(posedge swr) begin	
		if (saddress == A_address) begin
			A <= sdata_in;
			S <= 0;
			W <= 0;
			current_number <= 1;
			potential_factor <= 2;
			start <= 1;
		end
	end

	// implementacja automatu (maszyny stanow)

	// logika stanow
	always @(posedge clk) begin
		case(state_reg)
		
			idle: begin
				if(start) begin
					state_reg <= is_ready;
				end
				else begin
					state_reg <= idle;
				end    
			end
			
			is_ready: begin
				if(S < A) begin
					state_reg <= check_if_factor;
				end
				else begin
					state_reg <= final_state;
				end
			end

			check_if_factor: begin         
				if(potential_factor < current_number / 2 + 1) begin 
					if (current_number % potential_factor == 0) begin
						state_reg <= is_ready;
					end
					else begin 
						state_reg <= check_if_factor; 
					end 
				end
				else begin
					state_reg <= increment_status;
				end
			end

			increment_status: begin
				state_reg <= is_ready;
			end
			
			final_state: begin
				state_reg <= idle;
			end
			
			default: state_reg <= idle;
			
		endcase
	end
	
	// logika mikrooperacji
	always @(posedge clk) begin
		case(state_reg)
			
			is_ready: begin
				if(S < A) begin
					current_number <= current_number + 1;
				end
				start <= 0;
			end

			check_if_factor: begin
				if(potential_factor < current_number / 2 + 1) begin 
					if (current_number % potential_factor == 0) begin
						potential_factor <= 2;
					end
					else begin 
						potential_factor <= potential_factor + 1;
					end 
				end
			end

			increment_status: begin
					S <= S + 1;
			end
			
			final_state: begin
				W <= current_number;
			end
			
		endcase
	end
	
	// inkrementacja wyjscia gpio
	always@(posedge clk) begin
		if(state_reg == final_state) gpio_out_s <= gpio_out_s + 1;
	end
	
	assign gpio_out = gpio_out_s;
	assign gpio_in_s_insp = gpio_in_s;
	assign sdata_out = sdata_out_s;

endmodule

