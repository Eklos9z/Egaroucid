# Egaroucid 6.3.0 Benchmarks

## The FFO endgame test suite

<a href="http://radagast.se/othello/ffotest.html" target="_blank" el=”noopener noreferrer”>The FFO endgame test suite</a> is a common benchmark for endgame searching. Computer completely solves each testcase, and find the best move. This benchmark evaluates the exact time for searching and the speed (NPS: Nodes Per Second).

I used Core i9-11900K for testing.

### Egaroucid for Console 6.3.0 Windows x64 SIMD

In this testcase, Egaroucid is faster than Edax 4.4

#### Core i9 11900K

Used 19 threads

<table>
<tr>
<th>No.</th>
<th>Depth</th>
<th>Best Move</th>
<th>Score</th>
<th>Time (sec)</th>
<th>Nodes</th>
<th>NPS</th>
</tr>
<tr>
<td>#40</td>
<td>20@100%</td>
<td>a2</td>
<td>+38</td>
<td>0.049</td>
<td>16598652</td>
<td>338748000</td>
</tr>
<tr>
<td>#41</td>
<td>22@100%</td>
<td>h4</td>
<td>+0</td>
<td>0.078</td>
<td>21747252</td>
<td>278810923</td>
</tr>
<tr>
<td>#42</td>
<td>22@100%</td>
<td>g2</td>
<td>+6</td>
<td>0.151</td>
<td>50850284</td>
<td>336756847</td>
</tr>
<tr>
<td>#43</td>
<td>23@100%</td>
<td>g3</td>
<td>-12</td>
<td>0.293</td>
<td>88445051</td>
<td>301860242</td>
</tr>
<tr>
<td>#44</td>
<td>23@100%</td>
<td>b8</td>
<td>-14</td>
<td>0.098</td>
<td>21826343</td>
<td>222717785</td>
</tr>
<tr>
<td>#45</td>
<td>24@100%</td>
<td>b2</td>
<td>+6</td>
<td>0.975</td>
<td>406336171</td>
<td>416755047</td>
</tr>
<tr>
<td>#46</td>
<td>24@100%</td>
<td>b3</td>
<td>-8</td>
<td>0.218</td>
<td>67074299</td>
<td>307680270</td>
</tr>
<tr>
<td>#47</td>
<td>25@100%</td>
<td>g2</td>
<td>+4</td>
<td>0.101</td>
<td>18726289</td>
<td>185408801</td>
</tr>
<tr>
<td>#48</td>
<td>25@100%</td>
<td>f6</td>
<td>+28</td>
<td>0.74</td>
<td>165870291</td>
<td>224149041</td>
</tr>
<tr>
<td>#49</td>
<td>26@100%</td>
<td>e1</td>
<td>+16</td>
<td>0.685</td>
<td>187065270</td>
<td>273087985</td>
</tr>
<tr>
<td>#50</td>
<td>26@100%</td>
<td>d8</td>
<td>+10</td>
<td>4.368</td>
<td>1190461993</td>
<td>272541665</td>
</tr>
<tr>
<td>#51</td>
<td>27@100%</td>
<td>e2</td>
<td>+6</td>
<td>1.544</td>
<td>492558593</td>
<td>319014632</td>
</tr>
<tr>
<td>#52</td>
<td>27@100%</td>
<td>a3</td>
<td>+0</td>
<td>1.359</td>
<td>402202878</td>
<td>295955024</td>
</tr>
<tr>
<td>#53</td>
<td>28@100%</td>
<td>d8</td>
<td>-2</td>
<td>7.906</td>
<td>2858113187</td>
<td>361511913</td>
</tr>
<tr>
<td>#54</td>
<td>28@100%</td>
<td>c7</td>
<td>-2</td>
<td>11.145</td>
<td>4031223396</td>
<td>361706899</td>
</tr>
<tr>
<td>#55</td>
<td>29@100%</td>
<td>g6</td>
<td>+0</td>
<td>30.522</td>
<td>9313861679</td>
<td>305152404</td>
</tr>
<tr>
<td>#56</td>
<td>29@100%</td>
<td>h5</td>
<td>+2</td>
<td>3.927</td>
<td>789099800</td>
<td>200942144</td>
</tr>
<tr>
<td>#57</td>
<td>30@100%</td>
<td>a6</td>
<td>-10</td>
<td>5.891</td>
<td>1561827228</td>
<td>265120901</td>
</tr>
<tr>
<td>#58</td>
<td>30@100%</td>
<td>g1</td>
<td>+4</td>
<td>4.673</td>
<td>1140596937</td>
<td>244082374</td>
</tr>
<tr>
<td>#59</td>
<td>34@100%</td>
<td>e8</td>
<td>+64</td>
<td>0.298</td>
<td>6845851</td>
<td>22972654</td>
</tr>
<tr>
<td>All</td>
<td>-</td>
<td>-</td>
<td>-</td>
<td>75.021</td>
<td>22831331444</td>
<td>304332539</td>
</tr>
</table>

#### Core i9 13900K

Used 32 threads

<table>
<tr>
<th>No.</th>
<th>Depth</th>
<th>Best Move</th>
<th>Score</th>
<th>Time (sec)</th>
<th>Nodes</th>
<th>NPS</th>
</tr>
<tr>
<td>#40</td>
<td>20@100%</td>
<td>a2</td>
<td>+38</td>
<td>0.035</td>
<td>16419096</td>
<td>469117028</td>
</tr>
<tr>
<td>#41</td>
<td>22@100%</td>
<td>h4</td>
<td>+0</td>
<td>0.065</td>
<td>22485295</td>
<td>345927615</td>
</tr>
<tr>
<td>#42</td>
<td>22@100%</td>
<td>g2</td>
<td>+6</td>
<td>0.124</td>
<td>53707185</td>
<td>433122459</td>
</tr>
<tr>
<td>#43</td>
<td>23@100%</td>
<td>g3</td>
<td>-12</td>
<td>0.2</td>
<td>89644386</td>
<td>448221930</td>
</tr>
<tr>
<td>#44</td>
<td>23@100%</td>
<td>b8</td>
<td>-14</td>
<td>0.091</td>
<td>25322904</td>
<td>278273670</td>
</tr>
<tr>
<td>#45</td>
<td>24@100%</td>
<td>b2</td>
<td>+6</td>
<td>0.532</td>
<td>385489490</td>
<td>724604304</td>
</tr>
<tr>
<td>#46</td>
<td>24@100%</td>
<td>b3</td>
<td>-8</td>
<td>0.179</td>
<td>74263461</td>
<td>414879670</td>
</tr>
<tr>
<td>#47</td>
<td>25@100%</td>
<td>g2</td>
<td>+4</td>
<td>0.097</td>
<td>19807151</td>
<td>204197432</td>
</tr>
<tr>
<td>#48</td>
<td>25@100%</td>
<td>f6</td>
<td>+28</td>
<td>0.483</td>
<td>165131562</td>
<td>341887291</td>
</tr>
<tr>
<td>#49</td>
<td>26@100%</td>
<td>e1</td>
<td>+16</td>
<td>0.472</td>
<td>222527534</td>
<td>471456639</td>
</tr>
<tr>
<td>#50</td>
<td>26@100%</td>
<td>d8</td>
<td>+10</td>
<td>2.488</td>
<td>1209087179</td>
<td>485967515</td>
</tr>
<tr>
<td>#51</td>
<td>27@100%</td>
<td>e2</td>
<td>+6</td>
<td>1.127</td>
<td>600009904</td>
<td>532395655</td>
</tr>
<tr>
<td>#52</td>
<td>27@100%</td>
<td>a3</td>
<td>+0</td>
<td>1.016</td>
<td>439238408</td>
<td>432321267</td>
</tr>
<tr>
<td>#53</td>
<td>28@100%</td>
<td>d8</td>
<td>-2</td>
<td>5.134</td>
<td>3343344683</td>
<td>651216338</td>
</tr>
<tr>
<td>#54</td>
<td>28@100%</td>
<td>c7</td>
<td>-2</td>
<td>7.075</td>
<td>4404048943</td>
<td>622480415</td>
</tr>
<tr>
<td>#55</td>
<td>29@100%</td>
<td>g6</td>
<td>+0</td>
<td>18.67</td>
<td>9828917587</td>
<td>526455146</td>
</tr>
<tr>
<td>#56</td>
<td>29@100%</td>
<td>h5</td>
<td>+2</td>
<td>2.586</td>
<td>816990912</td>
<td>315928426</td>
</tr>
<tr>
<td>#57</td>
<td>30@100%</td>
<td>a6</td>
<td>-10</td>
<td>4.026</td>
<td>1969371260</td>
<td>489163253</td>
</tr>
<tr>
<td>#58</td>
<td>30@100%</td>
<td>g1</td>
<td>+4</td>
<td>3.434</td>
<td>1269941047</td>
<td>369813933</td>
</tr>
<tr>
<td>#59</td>
<td>34@100%</td>
<td>e8</td>
<td>+64</td>
<td>0.486</td>
<td>7763042</td>
<td>15973337</td>
</tr>
<tr>
<td>All</td>
<td>-</td>
<td>-</td>
<td>-</td>
<td>48.32</td>
<td>24963511029</td>
<td>516628953</td>
</tr>
</table>





## Play against Edax 4.4

<a href="https://github.com/abulmo/edax-reversi" target="_blank" el=”noopener noreferrer”>Edax 4.4</a> is one of the best Othello AI in the world.

If I set the game from the very beginning, same line appears a lot. To avoid this, I set the game from many different near-draw lines.

I used <a href="https://berg.earthlingz.de/xot/index.php" target="_blank" el=”noopener noreferrer”>XOT</a> for its testcases.

No opening books used.

If Egaroucid Win Ratio is over 0.5, then Egaroucid wins more than Edax do. "Black" and "White" means Egaroucid played black/white. In all conditions, Egaroucid is stronger than Edax.

<table>
<tr>
<th>Level</th>
<th>Egaroucid win</th>
<th>Draw</th>
<th>Edax Win</th>
<th>Egaroucid Win Ratio</th>
</tr>
<tr>
<td>1</td>
<td>1227(Black: 591 White: 636)</td>
<td>46(Black: 20 White: 26)</td>
<td>727(Black: 389 White: 338)</td>
<td>0.628</td>
</tr>
<tr>
<td>5</td>
<td>1154(Black: 593 White: 561)</td>
<td>87(Black: 45 White: 42)</td>
<td>759(Black: 362 White: 397)</td>
<td>0.603</td>
</tr>
<tr>
<td>10</td>
<td>1050(Black: 599 White: 451)</td>
<td>237(Black: 107 White: 130)</td>
<td>713(Black: 294 White: 419)</td>
<td>0.596</td>
</tr>
<tr>
<td>15</td>
<td>302(Black: 162 White: 140)</td>
<td>114(Black: 51 White: 63)</td>
<td>184(Black: 87 White: 97)</td>
<td>0.621</td>
</tr>
</table>


