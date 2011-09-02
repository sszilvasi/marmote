library verilog;
use verilog.vl_types.all;
entity CMMaster3Stage is
    port(
        HCLK            : in     vl_logic;
        HRESETn         : in     vl_logic;
        F2_ESRAMSIZE    : in     vl_logic_vector(1 downto 0);
        COM_MASTERENABLE: in     vl_logic;
        COM_CLEARSTATUS : in     vl_logic;
        COM_ERRORSTATUS : out    vl_logic;
        HADDR           : in     vl_logic_vector(31 downto 0);
        HMASTLOCK       : in     vl_logic;
        HSIZE           : in     vl_logic_vector(2 downto 0);
        HTRANS1         : in     vl_logic;
        HWRITE          : in     vl_logic;
        HRESP           : out    vl_logic;
        HRDATA          : out    vl_logic_vector(31 downto 0);
        HREADY_M        : out    vl_logic;
        sAddrReady      : in     vl_logic_vector(7 downto 0);
        sDataReady      : in     vl_logic_vector(7 downto 0);
        sHResp          : in     vl_logic_vector(7 downto 0);
        gatedHADDR      : out    vl_logic_vector(31 downto 0);
        gatedHMASTLOCK  : out    vl_logic;
        gatedHSIZE      : out    vl_logic_vector(2 downto 0);
        gatedHTRANS1    : out    vl_logic;
        gatedHWRITE     : out    vl_logic;
        sAddrSel        : out    vl_logic_vector(7 downto 0);
        sDataSel        : out    vl_logic_vector(7 downto 0);
        prevDataSlaveReady: out    vl_logic;
        HRDATA_S0       : in     vl_logic_vector(31 downto 0);
        HREADYOUT_S0    : in     vl_logic;
        HRDATA_S1       : in     vl_logic_vector(31 downto 0);
        HREADYOUT_S1    : in     vl_logic;
        HRDATA_S3       : in     vl_logic_vector(31 downto 0);
        HREADYOUT_S3    : in     vl_logic;
        HRDATA_S5       : in     vl_logic_vector(31 downto 0);
        HREADYOUT_S5    : in     vl_logic
    );
end CMMaster3Stage;
