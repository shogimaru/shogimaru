#include "../sfen.h"
#include <QDebug>
#include <QTest>
#include <QTranslator>


class MaruTest : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void testSfen();
    void testMove_data();
    void testMove();
    void testSfenParser_data();
    void testSfenParser();
    void testFromCsa_data();
    void testFromCsa();
    void testFromKif_data();
    void testFromKif();
    void testGenerateKifJa_data();
    void testGenerateKifJa();
    void benchmarkSfen();
};


void MaruTest::initTestCase()
{
}


void MaruTest::testSfen()
{
    QCOMPARE(Sfen().toSfen(), QByteArray());
    QCOMPARE(Sfen(Sfen::defaultPostion()).toSfen(), "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
}


void MaruTest::testMove_data()
{
    QTest::addColumn<QByteArray>("sfen");
    QTest::addColumn<QByteArray>("moves");
    QTest::addColumn<QByteArray>("expect");

    QTest::newRow("1") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1")
                       << QByteArray("7g7f")
                       << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/2P6/PP1PPPPPP/1B5R1/LNSGKGSNL w - 2");
    QTest::newRow("2") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1")
                       << QByteArray("2g2f 4a3b 7g7f 7a6b 2f2e 8c8d 6i7h")
                       << QByteArray("ln1gk1snl/1r1s2gb1/p1ppppppp/1p7/7P1/2P6/PP1PPPP1P/1BG4R1/LNS1KGSNL w - 8");
    QTest::newRow("3") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1")
                       << QByteArray("2g2f 4a3b 7g7f 7a6b 2f2e 8c8d 6i7h 1c1d 1g1f 3c3d 8h2b+")
                       << QByteArray("ln1gk1snl/1r1s2g+B1/p1pppp1p1/1p4p1p/7P1/2P5P/PP1PPPP2/2G4R1/LNS1KGSNL w B 12");
    QTest::newRow("4") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1")
                       << QByteArray("2g2f 4a3b 7g7f 7a6b 2f2e 8c8d 6i7h 1c1d 1g1f 3c3d 8h2b+ 3a2b 7i8h 2b3c 3i3h 6c6d 3g3f 7c7d 2i3g 6a7b")
                       << QByteArray("ln2k2nl/1rgs2g2/p3ppsp1/1ppp2p1p/7P1/2P3P1P/PP1PPPN2/1SG3SR1/LN2KG2L b Bb 21");
    QTest::newRow("5") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1")
                       << QByteArray("2g2f 4a3b 7g7f 7a6b 2f2e 8c8d 6i7h 1c1d 1g1f 3c3d 8h2b+ 3a2b 7i8h 2b3c 3i3h 6c6d 3g3f 7c7d 2i3g 6a7b 8h7g 6b6c 4g4f 8d8e 3g4e 3c4b 2e2d 2c2d")
                       << QByteArray("ln2k2nl/1rg2sg2/p2spp3/2pp2ppp/1p3N3/2P2PP1P/PPSPP4/2G3SR1/LN2KG2L b Bbp 29");
    QTest::newRow("6") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1")
                       << QByteArray("2g2f 4a3b 7g7f 7a6b 2f2e 8c8d 6i7h 1c1d 1g1f 3c3d 8h2b+ 3a2b 7i8h 2b3c 3i3h 6c6d 3g3f 7c7d 2i3g 6a7b 8h7g 6b6c 4g4f 8d8e 3g4e 3c4b 2e2d 2c2d 2h2d P*2c 2d3d 3b2b B*3a B*4d 3a2b+ 8e8f 2b4d 8f8g+")
                       << QByteArray("ln2k2nl/1rg2s3/p2spp1p1/2pp1+BR1p/5N3/2P2PP1P/P+pSPP4/2G3S2/LN2KG2L b BG2Pp 39");
    QTest::newRow("7") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1")
                       << QByteArray("2g2f 4a3b 2f2e 7a6b 7g7f 8c8d 6i7h 3c3d 3i4h 8d8e 2e2d 2c2d 2h2d 5a4b 9g9f 8e8f 8g8f 2b8h+ 7i8h 3a2b 2d2h 8b8f 5i6h P*2g 2h2g 8f8e P*8g 4c4d 8h7g 8e8b 3g3f P*2d 2g2d 2a3c 2i3g 1c1d 1g1f 6a5b 4g4f 5b4c 4h4g B*9b 4i4h 2b2c 2d2h 9c9d 6h7i 2c1b 1f1e 1d1e 1i1e P*8h 7i8h 9b4g+ 4h4g S*6i 7h6h 6i5h+ 6h5h 5c5d 1e1b+ 8b9b 2h2a+ 6b5a 1b1a 9b5b B*6a 5b6b 6a4c+ 3b4c B*3a 4b5b L*5c 5b6a 5c5a+ 6a7b 3a7e+ B*4i 7e8d 6b4b S*8c 7b6b 5a6a 6b5c 2a5a 4b5b 8d7e 6c6d S*6b 5c6c S*7b")
                       << QByteArray("ln1+L+R3+L/2SSr4/1Spk1gn2/p2pppp2/2+B6/P1P2PP2/1PSPPGN2/1K2G4/LN3b3 w G5P 92");
}


void MaruTest::testMove()
{
    QFETCH(QByteArray, sfen);
    QFETCH(QByteArray, moves);
    QFETCH(QByteArray, expect);

    Sfen parser(sfen);
    parser.move(moves.split(' '));
    QCOMPARE(parser.toSfen(), expect);
}


void MaruTest::testSfenParser_data()
{
    QTest::addColumn<QByteArray>("sfen");

    QTest::newRow("1") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNM b - 1");  // Mがおかしい
    QTest::newRow("2") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL r - 1");  // 手番の文字が不正
    QTest::newRow("3") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 7f7e");  // 移動元に駒がない
    QTest::newRow("4") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 7g7f 1b1c");  // 移動元に駒がない
    QTest::newRow("5") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNLL b - 1");  // 1行の文字が多い
    QTest::newRow("6") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGS/NL b - 1");  // 行が多い
    QTest::newRow("7") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b Bt 1");  // 持ち駒の文字が不正
    QTest::newRow("8") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b -1 1");  // 持ち駒の文字が不正
}


void MaruTest::testSfenParser()
{
    QFETCH(QByteArray, sfen);
    bool ok;
    Sfen::fromSfen(sfen, &ok);
    QCOMPARE(ok, false);
}


void MaruTest::testFromCsa_data()
{
    QTest::addColumn<QString>("csa");
    QTest::addColumn<QByteArray>("expect");
    QTest::addColumn<QByteArray>("expectFinalSfen");

    QTest::newRow("1") << QString::fromLatin1("V2.2\nN+hoge\nN-foo\nP1-KY-KE-GI-KI-OU-KI-GI-KE-KY\nP2 * -HI *  *  *  *  * -KA * \nP3-FU-FU-FU-FU-FU-FU-FU-FU-FU\nP4 *  *  *  *  *  *  *  *  * \nP5 *  *  *  *  *  *  *  *  * \nP6 *  *  *  *  *  *  *  *  * \nP7+FU+FU+FU+FU+FU+FU+FU+FU+FU\nP8 * +KA *  *  *  *  * +HI * \nP9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n+\n+2878HI\n-3334FU\n+5948OU\n-8384FU\n+4838OU\n-5142OU\n+7776FU\n-8485FU\n+8877KA\n-2277UM\n+8977KE\n-4232OU\n+7888HI\n-1314FU\n+1716FU\n-7172GI\n+7968GI\n-4142KI\n+6766FU\n-6364FU\n+6978KI\n-7263GI\n+5756FU\n-6354GI\n+8889HI\n-6152KI\n+3948GI\n-2133KE\n+6867GI\n-9394FU\n+9796FU\n-4344FU\n+4857GI\n-5243KI\n+4948KI\n-4445FU\n+7675FU\n-3435FU\n+6776GI\n-9495FU\n+9695FU\n-8586FU\n+8786FU\n-9195KY\n+7687GI\n-0098FU\n+9998KY\n-9598NY\n+8798GI\n-8292HI\n+0097KY\n-9262HI\n+9887GI\n-0076KY\n+0083KA\n-7677KY\n+7877KI\n-0044KE\n+4858KI\n-1415FU\n+1615FU\n-3536FU\n+3736FU\n-4436KE\n+0037FU\n-0028KA\n+1917KY\n-3325KE\n+3736FU\n-2517NK\n+5868KI\n-1115KY\n+0014FU\n-3122GI\n+9792NY\n-6465FU\n+6665FU\n-4546FU\n+4746FU\n-0045FU\n+3848OU\n-4546FU\n+4858OU\n-0082FU\n+8394UM\n-6265HI\n+0048FU\n-0044KY\n+0035KE\n-6545HI\n+0049KY\n-0034FU\n+3543NK\n-5443GI\n+9281NY\n-1727NK\n+0055KE\n-4354GI\n+9461UM\n-3233OU\n+6151UM\n-0065KE\n+7766KI\n-6557NK\n+6857KI\n-0065FU\n+6665KI\n-3324OU\n+6554KI\n-5354FU\n+5142UM\n-0033GI\n+4243UM\n-5455FU\n+1413TO\n-0066FU\n+5766KI\n-2425OU\n+0057KE\n-4647TO\n+4847FU\n-2839UM\n+5867OU\n%TORYO\n")
                       << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2h7h 3c3d 5i4h 8c8d 4h3h 5a4b 7g7f 8d8e 8h7g 2b7g+ 8i7g 4b3b 7h8h 1c1d 1g1f 7a7b 7i6h 4a4b 6g6f 6c6d 6i7h 7b6c 5g5f 6c5d 8h8i 6a5b 3i4h 2a3c 6h6g 9c9d 9g9f 4c4d 4h5g 5b4c 4i4h 4d4e 7f7e 3d3e 6g7f 9d9e 9f9e 8e8f 8g8f 9a9e 7f8g P*9h 9i9h 9e9h+ 8g9h 8b9b L*9g 9b6b 9h8g L*7f B*8c 7f7g 7h7g N*4d 4h5h 1d1e 1f1e 3e3f 3g3f 4d3f P*3g B*2h 1i1g 3c2e 3g3f 2e1g+ 5h6h 1a1e P*1d 3a2b 9g9b+ 6d6e 6f6e 4e4f 4g4f P*4e 3h4h 4e4f 4h5h P*8b 8c9d+ 6b6e P*4h L*4d N*3e 6e4e L*4i P*3d 3e4c+ 5d4c 9b8a 1g2g N*5e 4c5d 9d6a 3b3c 6a5a N*6e 7g6f 6e5g+ 6h5g P*6e 6f6e 3c2d 6e5d 5c5d 5a4b S*3c 4b4c 5d5e 1d1c+ P*6f 5g6f 2d2e N*5g 4f4g+ 4h4g 2h3i+ 5h6g")
                       << QByteArray("1+L7/1p5s1/2p2+Bsp+P/5lp2/2P1pr1kl/1P1GP1P2/1S1KNP1+n1/9/1R3L+bN1 w 2GS7Pgn 124");
    QTest::newRow("2") << QString::fromLatin1("V2.2\nN+hoge\nN-foo\nP1-KY-KE-GI-KI-OU-KI-GI-KE-KY\nP2 * -HI *  *  *  *  * -KA * \nP3-FU-FU-FU-FU-FU-FU-FU-FU-FU\nP4 *  *  *  *  *  *  *  *  * \nP5 *  *  *  *  *  *  *  *  * \nP6 *  *  *  *  *  *  *  *  * \nP7+FU+FU+FU+FU+FU+FU+FU+FU+FU\nP8 * +KA *  *  *  *  * +HI * \nP9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n+\n+2726FU\n-8384FU\n+2625FU\n-8485FU\n+6978KI\n-4132KI\n+3938GI\n-7172GI\n+9796FU\n-9394FU\n+5968OU\n-1314FU\n+3736FU\n-8586FU\n+8786FU\n-8286HI\n+2937KE\n-7374FU\n+2524FU\n-2324FU\n+2824HI\n-0023FU\n+2474HI\n-7273GI\n+0087FU\n-8682HI\n+7475HI\n-3334FU\n+7525HI\n-3142GI\n+7776FU\n-4344FU\n+4746FU\n-4243GI\n+4948KI\n-7364GI\n+3847GI\n-4354GI\n+2529HI\n-5465GI\n+7877KI\n-0075FU\n+7675FU\n-6475GI\n+8897KA\n-7564GI\n+6766FU\n-6574GI\n+7978GI\n-2233KA\n+9788KA\n-7485GI\n+6858OU\n-6152KI\n+4756GI\n-5354FU\n+6665FU\n-6475GI\n+0024FU\n-2324FU\n+0025FU\n-5243KI\n+2524FU\n-0022FU\n+5667GI\n-3435FU\n+0076FU\n-7584GI\n+2926HI\n-3536FU\n+2636HI\n-0034FU\n+7766KI\n-3324KA\n+6656KI\n-3435FU\n+3616HI\n-0086FU\n+4645FU\n-8687TO\n+7887GI\n-0086FU\n+8778GI\n-4334KI\n+0083FU\n-8283HI\n+4544FU\n-3536FU\n+1636HI\n-3435KI\n+3635HI\n-2435KA\n+5645KI\n-3513KA\n+4554KI\n-0047FU\n+4838KI\n-5162OU\n+4443TO\n-3243KI\n+5443KI\n-6272OU\n+8997KE\n-0036FU\n+9785KE\n-3637TO\n+0062KI\n-7262OU\n+8844KA\n-6272OU\n+0061GI\n%TORYO\n")
                       << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2g2f 8c8d 2f2e 8d8e 6i7h 4a3b 3i3h 7a7b 9g9f 9c9d 5i6h 1c1d 3g3f 8e8f 8g8f 8b8f 2i3g 7c7d 2e2d 2c2d 2h2d P*2c 2d7d 7b7c P*8g 8f8b 7d7e 3c3d 7e2e 3a4b 7g7f 4c4d 4g4f 4b4c 4i4h 7c6d 3h4g 4c5d 2e2i 5d6e 7h7g P*7e 7f7e 6d7e 8h9g 7e6d 6g6f 6e7d 7i7h 2b3c 9g8h 7d8e 6h5h 6a5b 4g5f 5c5d 6f6e 6d7e P*2d 2c2d P*2e 5b4c 2e2d P*2b 5f6g 3d3e P*7f 7e8d 2i2f 3e3f 2f3f P*3d 7g6f 3c2d 6f5f 3d3e 3f1f P*8f 4f4e 8f8g+ 7h8g P*8f 8g7h 4c3d P*8c 8b8c 4e4d 3e3f 1f3f 3d3e 3f3e 2d3e 5f4e 3e1c 4e5d P*4g 4h3h 5a6b 4d4c+ 3b4c 5d4c 6b7b 8i9g P*3f 9g8e 3f3g+ G*6b 7b6b 8h4d 6b7b S*6a")
                       << QByteArray("ln1S3nl/2k4p1/1r1p1G2b/ps3B2p/1N1P5/PpP6/3SPp+p1P/2S1K1G2/L7L w G5Prgnp 112");
}


void MaruTest::testFromCsa()
{
    QFETCH(QString, csa);
    QFETCH(QByteArray, expect);
    QFETCH(QByteArray, expectFinalSfen);

    Sfen sfen = Sfen::fromCsa(csa);
    QCOMPARE(sfen.toUsi(), expect);
    QCOMPARE(sfen.toSfen(), expectFinalSfen);
}

void MaruTest::testFromKif_data()
{
    QTest::addColumn<QString>("kif");
    QTest::addColumn<QByteArray>("expect");
    QTest::addColumn<QByteArray>("expectFinalSfen");

    QTest::newRow("1") << QString::fromUtf8("手合割：平手\n先手：hoge\n後手：foo\n手数----指手---------消費時間--\n   1   ７八飛(28)\n   2   ３四歩(33)\n   3   ４八玉(59)\n   4   ８四歩(83)\n   5   ３八玉(48)\n   6   ４二玉(51)\n   7   ７六歩(77)\n   8   ８五歩(84)\n   9   ７七角(88)\n  10   同　角成(22)\n  11   同　桂(89)\n  12   ３二玉(42)\n  13   ８八飛(78)\n  14   １四歩(13)\n  15   １六歩(17)\n  16   ７二銀(71)\n  17   ６八銀(79)\n  18   ４二金(41)\n  19   ６六歩(67)\n  20   ６四歩(63)\n  21   ７八金(69)\n  22   ６三銀(72)\n  23   ５六歩(57)\n  24   ５四銀(63)\n  25   ８九飛(88)\n  26   ５二金(61)\n  27   ４八銀(39)\n  28   ３三桂(21)\n  29   ６七銀(68)\n  30   ９四歩(93)\n  31   ９六歩(97)\n  32   ４四歩(43)\n  33   ５七銀(48)\n  34   ４三金(52)\n  35   ４八金(49)\n  36   ４五歩(44)\n  37   ７五歩(76)\n  38   ３五歩(34)\n  39   ７六銀(67)\n  40   ９五歩(94)\n  41   同　歩(96)\n  42   ８六歩(85)\n  43   同　歩(87)\n  44   ９五香(91)\n  45   ８七銀(76)\n  46   ９八歩打\n  47   同　香(99)\n  48   同　香成(95)\n  49   同　銀(87)\n  50   ９二飛(82)\n  51   ９七香打\n  52   ６二飛(92)\n  53   ８七銀(98)\n  54   ７六香打\n  55   ８三角打\n  56   ７七香(76)\n  57   同　金(78)\n  58   ４四桂打\n  59   ５八金(48)\n  60   １五歩(14)\n  61   同　歩(16)\n  62   ３六歩(35)\n  63   同　歩(37)\n  64   同　桂(44)\n  65   ３七歩打\n  66   ２八角打\n  67   １七香(19)\n  68   ２五桂(33)\n  69   ３六歩(37)\n  70   １七桂成(25)\n  71   ６八金(58)\n  72   １五香(11)\n  73   １四歩打\n  74   ２二銀(31)\n  75   ９二香成(97)\n  76   ６五歩(64)\n  77   同　歩(66)\n  78   ４六歩(45)\n  79   同　歩(47)\n  80   ４五歩打\n  81   ４八玉(38)\n  82   ４六歩(45)\n  83   ５八玉(48)\n  84   ８二歩打\n  85   ９四角成(83)\n  86   ６五飛(62)\n  87   ４八歩打\n  88   ４四香打\n  89   ３五桂打\n  90   ４五飛(65)\n  91   ４九香打\n  92   ３四歩打\n  93   ４三桂成(35)\n  94   同　銀(54)\n  95   ８一成香(92)\n  96   ２七成桂(17)\n  97   ５五桂打\n  98   ５四銀(43)\n  99   ６一馬(94)\n 100   ３三玉(32)\n 101   ５一馬(61)\n 102   ６五桂打\n 103   ６六金(77)\n 104   ５七桂成(65)\n 105   同　金(68)\n 106   ６五歩打\n 107   同　金(66)\n 108   ２四玉(33)\n 109   ５四金(65)\n 110   同　歩(53)\n 111   ４二馬(51)\n 112   ３三銀打\n 113   ４三馬(42)\n 114   ５五歩(54)\n 115   １三歩成(14)\n 116   ６六歩打\n 117   同　金(57)\n 118   ２五玉(24)\n 119   ５七桂打\n 120   ４七歩成(46)\n 121   同　歩(48)\n 122   ３九角成(28)\n 123   ６七玉(58)\n 124   投了\n")
                       << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2h7h 3c3d 5i4h 8c8d 4h3h 5a4b 7g7f 8d8e 8h7g 2b7g+ 8i7g 4b3b 7h8h 1c1d 1g1f 7a7b 7i6h 4a4b 6g6f 6c6d 6i7h 7b6c 5g5f 6c5d 8h8i 6a5b 3i4h 2a3c 6h6g 9c9d 9g9f 4c4d 4h5g 5b4c 4i4h 4d4e 7f7e 3d3e 6g7f 9d9e 9f9e 8e8f 8g8f 9a9e 7f8g P*9h 9i9h 9e9h+ 8g9h 8b9b L*9g 9b6b 9h8g L*7f B*8c 7f7g 7h7g N*4d 4h5h 1d1e 1f1e 3e3f 3g3f 4d3f P*3g B*2h 1i1g 3c2e 3g3f 2e1g+ 5h6h 1a1e P*1d 3a2b 9g9b+ 6d6e 6f6e 4e4f 4g4f P*4e 3h4h 4e4f 4h5h P*8b 8c9d+ 6b6e P*4h L*4d N*3e 6e4e L*4i P*3d 3e4c+ 5d4c 9b8a 1g2g N*5e 4c5d 9d6a 3b3c 6a5a N*6e 7g6f 6e5g+ 6h5g P*6e 6f6e 3c2d 6e5d 5c5d 5a4b S*3c 4b4c 5d5e 1d1c+ P*6f 5g6f 2d2e N*5g 4f4g+ 4h4g 2h3i+ 5h6g")
                       << QByteArray("1+L7/1p5s1/2p2+Bsp+P/5lp2/2P1pr1kl/1P1GP1P2/1S1KNP1+n1/9/1R3L+bN1 w 2GS7Pgn 124");
    QTest::newRow("2") << QString::fromUtf8("手合割：平手\n先手：hoge\n後手：foo\n手数----指手---------消費時間--\n   1   ２六歩(27)\n   2   ８四歩(83)\n   3   ２五歩(26)\n   4   ８五歩(84)\n   5   ７八金(69)\n   6   ３二金(41)\n   7   ３八銀(39)\n   8   ７二銀(71)\n   9   ９六歩(97)\n  10   ９四歩(93)\n  11   ６八玉(59)\n  12   １四歩(13)\n  13   ３六歩(37)\n  14   ８六歩(85)\n  15   同　歩(87)\n  16   同　飛(82)\n  17   ３七桂(29)\n  18   ７四歩(73)\n  19   ２四歩(25)\n  20   同　歩(23)\n  21   同　飛(28)\n  22   ２三歩打\n  23   ７四飛(24)\n  24   ７三銀(72)\n  25   ８七歩打\n  26   ８二飛(86)\n  27   ７五飛(74)\n  28   ３四歩(33)\n  29   ２五飛(75)\n  30   ４二銀(31)\n  31   ７六歩(77)\n  32   ４四歩(43)\n  33   ４六歩(47)\n  34   ４三銀(42)\n  35   ４八金(49)\n  36   ６四銀(73)\n  37   ４七銀(38)\n  38   ５四銀(43)\n  39   ２九飛(25)\n  40   ６五銀(54)\n  41   ７七金(78)\n  42   ７五歩打\n  43   同　歩(76)\n  44   同　銀(64)\n  45   ９七角(88)\n  46   ６四銀(75)\n  47   ６六歩(67)\n  48   ７四銀(65)\n  49   ７八銀(79)\n  50   ３三角(22)\n  51   ８八角(97)\n  52   ８五銀(74)\n  53   ５八玉(68)\n  54   ５二金(61)\n  55   ５六銀(47)\n  56   ５四歩(53)\n  57   ６五歩(66)\n  58   ７五銀(64)\n  59   ２四歩打\n  60   同　歩(23)\n  61   ２五歩打\n  62   ４三金(52)\n  63   ２四歩(25)\n  64   ２二歩打\n  65   ６七銀(56)\n  66   ３五歩(34)\n  67   ７六歩打\n  68   ８四銀(75)\n  69   ２六飛(29)\n  70   ３六歩(35)\n  71   同　飛(26)\n  72   ３四歩打\n  73   ６六金(77)\n  74   ２四角(33)\n  75   ５六金(66)\n  76   ３五歩(34)\n  77   １六飛(36)\n  78   ８六歩打\n  79   ４五歩(46)\n  80   ８七歩成(86)\n  81   同　銀(78)\n  82   ８六歩打\n  83   ７八銀(87)\n  84   ３四金(43)\n  85   ８三歩打\n  86   同　飛(82)\n  87   ４四歩(45)\n  88   ３六歩(35)\n  89   同　飛(16)\n  90   ３五金(34)\n  91   同　飛(36)\n  92   同　角(24)\n  93   ４五金(56)\n  94   １三角(35)\n  95   ５四金(45)\n  96   ４七歩打\n  97   ３八金(48)\n  98   ６二玉(51)\n  99   ４三歩成(44)\n 100   同　金(32)\n 101   同　金(54)\n 102   ７二玉(62)\n 103   ９七桂(89)\n 104   ３六歩打\n 105   ８五桂(97)\n 106   ３七歩成(36)\n 107   ６二金打\n 108   同　玉(72)\n 109   ４四角(88)\n 110   ７二玉(62)\n 111   ６一銀打\n 112   投了\n")
                       << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2g2f 8c8d 2f2e 8d8e 6i7h 4a3b 3i3h 7a7b 9g9f 9c9d 5i6h 1c1d 3g3f 8e8f 8g8f 8b8f 2i3g 7c7d 2e2d 2c2d 2h2d P*2c 2d7d 7b7c P*8g 8f8b 7d7e 3c3d 7e2e 3a4b 7g7f 4c4d 4g4f 4b4c 4i4h 7c6d 3h4g 4c5d 2e2i 5d6e 7h7g P*7e 7f7e 6d7e 8h9g 7e6d 6g6f 6e7d 7i7h 2b3c 9g8h 7d8e 6h5h 6a5b 4g5f 5c5d 6f6e 6d7e P*2d 2c2d P*2e 5b4c 2e2d P*2b 5f6g 3d3e P*7f 7e8d 2i2f 3e3f 2f3f P*3d 7g6f 3c2d 6f5f 3d3e 3f1f P*8f 4f4e 8f8g+ 7h8g P*8f 8g7h 4c3d P*8c 8b8c 4e4d 3e3f 1f3f 3d3e 3f3e 2d3e 5f4e 3e1c 4e5d P*4g 4h3h 5a6b 4d4c+ 3b4c 5d4c 6b7b 8i9g P*3f 9g8e 3f3g+ G*6b 7b6b 8h4d 6b7b S*6a")
                       << QByteArray("ln1S3nl/2k4p1/1r1p1G2b/ps3B2p/1N1P5/PpP6/3SPp+p1P/2S1K1G2/L7L w G5Prgnp 112");
}

void MaruTest::testFromKif()
{
    QFETCH(QString, kif);
    QFETCH(QByteArray, expect);
    QFETCH(QByteArray, expectFinalSfen);

    Sfen sfen = Sfen::fromKif(kif);
    QCOMPARE(sfen.toUsi(), expect);
    QCOMPARE(sfen.toSfen(), expectFinalSfen);
}

void MaruTest::testGenerateKifJa_data()
{
    QTest::addColumn<QByteArray>("sfen");
    QTest::addColumn<QByteArrayList>("moves");
    QTest::addColumn<QStringList>("expect");

    if (QLocale::system().name().toLower() == "ja_jp") {
        QTest::newRow("1") << QByteArray("ln1gk1snl/1r1s2g2/p1pppp1pp/1p4pP1/9/2P6/PP1PPPP1P/1+bG4R1/LNS1KGSNL b b 11")
                           << QByteArrayList({"2d2c+", "8h9i", "2c3b", "3a3b", "7i8h", "9i8i", "2h2b+"})
                           << QStringList({u8"▲２三歩成", u8"△９九馬", u8"▲３二と", u8"△同銀", u8"▲８八銀", u8"△８九馬", u8"▲２二飛成"});
        QTest::newRow("2") << QByteArray("ln1gk1snl/1r1s2g2/p1pppp1pp/1p4pP1/9/2P6/PP1PPPP1P/1+bG4R1/LNS1KGSNL b b 11")
                           << QByteArrayList({"2d2c+", "3b2c", "2h2c+", "8h9i"})
                           << QStringList({u8"▲２三歩成", u8"△同金", u8"▲同飛成", u8"△９九馬"});
        QTest::newRow("3") << QByteArray("ln1gk1snl/1r1s2g2/p1pppp1pp/1p4pP1/9/2P6/PP1PPPP1P/1+bG4R1/LNS1KGSNL b b 11")
                           << QByteArrayList({"2d2c+", "3b2c", "7i8h", "3a2b", "2h2c+"})
                           << QStringList({u8"▲２三歩成", u8"△同金", u8"▲８八銀", u8"△２二銀", u8"▲２三飛成"});
        QTest::newRow("4") << QByteArray("lnsgkg2l/1r4sB1/ppppppnpp/6p2/9/2P6/PPBPPPPPP/2G4R1/LNS1KGSNL b - 9")
                           << QByteArrayList({"2b1a+", "4a4b", "1a1b", "9c9d", "2g2f", "8b5b", "2f2e"})
                           << QStringList({u8"▲１一角成", u8"△４二金", u8"▲１二馬", u8"△９四歩", u8"▲２六歩", u8"△５二飛", u8"▲２五歩"});
    } else {
        QTest::newRow("11") << QByteArray("ln1gk1snl/1r1s2g2/p1pppp1pp/1p4pP1/9/2P6/PP1PPPP1P/1+bG4R1/LNS1KGSNL b b 11")
                            << QByteArrayList({"2d2c+", "8h9i", "2c3b", "3a3b", "7i8h", "9i8i", "2h2b+"})
                            << QStringList({u8"▲P23+", u8"△+B99", u8"▲+P32", u8"△Sx32", u8"▲S88", u8"△+B89", u8"▲R22+"});
        QTest::newRow("12") << QByteArray("ln1gk1snl/1r1s2g2/p1pppp1pp/1p4pP1/9/2P6/PP1PPPP1P/1+bG4R1/LNS1KGSNL b b 11")
                            << QByteArrayList({"2d2c+", "3b2c", "2h2c+", "8h9i"})
                            << QStringList({u8"▲P23+", u8"△Gx23", u8"▲Rx23+", u8"△+B99"});
        QTest::newRow("13") << QByteArray("ln1gk1snl/1r1s2g2/p1pppp1pp/1p4pP1/9/2P6/PP1PPPP1P/1+bG4R1/LNS1KGSNL b b 11")
                            << QByteArrayList({"2d2c+", "3b2c", "7i8h", "3a2b", "2h2c+"})
                            << QStringList({u8"▲P23+", u8"△Gx23", u8"▲S88", u8"△S22", u8"▲R23+"});
        QTest::newRow("14") << QByteArray("lnsgkg2l/1r4sB1/ppppppnpp/6p2/9/2P6/PPBPPPPPP/2G4R1/LNS1KGSNL b - 9")
                            << QByteArrayList({"2b1a+", "4a4b", "1a1b", "9c9d", "2g2f", "8b5b", "2f2e"})
                            << QStringList({u8"▲B11+", u8"△G42", u8"▲+B12", u8"△P94", u8"▲P26", u8"△R52", u8"▲P25"});
    }
}


void MaruTest::testGenerateKifJa()
{
    QFETCH(QByteArray, sfen);
    QFETCH(QByteArrayList, moves);
    QFETCH(QStringList, expect);

    QTranslator translator;
    QString ts = QString("message_") + QLocale::system().name();
    if (translator.load(ts, "../../assets/translations/")) {
        qApp->installTranslator(&translator);
    } else {
        qCritical() << "Error load translation file.";
    }

    Sfen sf(sfen);
    QStringList result = sf.generateKif(moves);
    qDebug() << result.join(" ");
    QCOMPARE(result, expect);

    qApp->removeTranslator(&translator);
}


void MaruTest::benchmarkSfen()
{
    const QByteArray csa("V2.2\nN+hoge\nN-foo\nP1-KY-KE-GI-KI-OU-KI-GI-KE-KY\nP2 * -HI *  *  *  *  * -KA * \nP3-FU-FU-FU-FU-FU-FU-FU-FU-FU\nP4 *  *  *  *  *  *  *  *  * \nP5 *  *  *  *  *  *  *  *  * \nP6 *  *  *  *  *  *  *  *  * \nP7+FU+FU+FU+FU+FU+FU+FU+FU+FU\nP8 * +KA *  *  *  *  * +HI * \nP9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n+\n+2878HI\n-3334FU\n+5948OU\n-8384FU\n+4838OU\n-5142OU\n+7776FU\n-8485FU\n+8877KA\n-2277UM\n+8977KE\n-4232OU\n+7888HI\n-1314FU\n+1716FU\n-7172GI\n+7968GI\n-4142KI\n+6766FU\n-6364FU\n+6978KI\n-7263GI\n+5756FU\n-6354GI\n+8889HI\n-6152KI\n+3948GI\n-2133KE\n+6867GI\n-9394FU\n+9796FU\n-4344FU\n+4857GI\n-5243KI\n+4948KI\n-4445FU\n+7675FU\n-3435FU\n+6776GI\n-9495FU\n+9695FU\n-8586FU\n+8786FU\n-9195KY\n+7687GI\n-0098FU\n+9998KY\n-9598NY\n+8798GI\n-8292HI\n+0097KY\n-9262HI\n+9887GI\n-0076KY\n+0083KA\n-7677KY\n+7877KI\n-0044KE\n+4858KI\n-1415FU\n+1615FU\n-3536FU\n+3736FU\n-4436KE\n+0037FU\n-0028KA\n+1917KY\n-3325KE\n+3736FU\n-2517NK\n+5868KI\n-1115KY\n+0014FU\n-3122GI\n+9792NY\n-6465FU\n+6665FU\n-4546FU\n+4746FU\n-0045FU\n+3848OU\n-4546FU\n+4858OU\n-0082FU\n+8394UM\n-6265HI\n+0048FU\n-0044KY\n+0035KE\n-6545HI\n+0049KY\n-0034FU\n+3543NK\n-5443GI\n+9281NY\n-1727NK\n+0055KE\n-4354GI\n+9461UM\n-3233OU\n+6151UM\n-0065KE\n+7766KI\n-6557NK\n+6857KI\n-0065FU\n+6665KI\n-3324OU\n+6554KI\n-5354FU\n+5142UM\n-0033GI\n+4243UM\n-5455FU\n+1413TO\n-0066FU\n+5766KI\n-2425OU\n+0057KE\n-4647TO\n+4847FU\n-2839UM\n+5867OU\n%TORYO\n");

    QBENCHMARK
    {
        Sfen::fromCsa(csa).toSfen();
    }
}


QTEST_GUILESS_MAIN(MaruTest)
#include "main.moc"
