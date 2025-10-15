./head.pl gds_render_preview.svg > a
./poly.pl gds_render_preview.svg > b
head -n 1 b > b1
tail -n +2 b > b2
grep "use transform" gds_render_preview.svg > c
tail -n 2 gds_render_preview.svg > d
cat a b1 c b2 d > gds_wires_and_cells.svg
grep clkbuf_8 c > clk
grep dly c > dly
grep df c > ffs
grep -v clk c | grep -v dly | grep -v df | grep -v decap | grep -v tap | grep -v fill > cells
cat a cells d > cells.svg
cat a clk d > clk.svg
cat a dly d > dly.svg
cat a ffs d > ffs.svg
rm a b b1 b2 c d cells clk dly ffs
