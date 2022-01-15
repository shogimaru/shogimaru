#include "../sfen.h"
#include <QTest>
#include <QDebug>


class MaruTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void testSfen();
    void testMove_data();
    void testMove();
    void testSfenParser_data();
    void testSfenParser();
    void testFromCsa_data();
    void testFromCsa();
    void testGenerateKif_data();
    void testGenerateKif();
    void benchmarkSfen();
};


void MaruTest::testSfen()
{
    Sfen sfen;
    auto result = sfen.toSfen();
    QCOMPARE(result, "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
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

    QTest::newRow("1") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNM b - 1"); // Mがおかしい
    QTest::newRow("2") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL r - 1"); // 手番の文字が不正
    QTest::newRow("3") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 7f7e"); // 移動元に駒がない
    QTest::newRow("4") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 7g7f 1b1c"); // 移動元に駒がない
    QTest::newRow("5") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNLL b - 1"); // 1行の文字が多い
    QTest::newRow("6") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGS/NL b - 1"); // 行が多い
    QTest::newRow("7") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b Bt 1"); // 持ち駒の文字が不正
    QTest::newRow("8") << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b -1 1"); // 持ち駒の文字が不正
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
    QTest::addColumn<QByteArray>("csa");
    QTest::addColumn<QByteArray>("expect");
    QTest::addColumn<QByteArray>("expectFinalSfen");

    QTest::newRow("1") << QByteArray("V2.2\nN+hoge\nN-foo\nP1-KY-KE-GI-KI-OU-KI-GI-KE-KY\nP2 * -HI *  *  *  *  * -KA * \nP3-FU-FU-FU-FU-FU-FU-FU-FU-FU\nP4 *  *  *  *  *  *  *  *  * \nP5 *  *  *  *  *  *  *  *  * \nP6 *  *  *  *  *  *  *  *  * \nP7+FU+FU+FU+FU+FU+FU+FU+FU+FU\nP8 * +KA *  *  *  *  * +HI * \nP9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n+\n+2878HI\n-3334FU\n+5948OU\n-8384FU\n+4838OU\n-5142OU\n+7776FU\n-8485FU\n+8877KA\n-2277UM\n+8977KE\n-4232OU\n+7888HI\n-1314FU\n+1716FU\n-7172GI\n+7968GI\n-4142KI\n+6766FU\n-6364FU\n+6978KI\n-7263GI\n+5756FU\n-6354GI\n+8889HI\n-6152KI\n+3948GI\n-2133KE\n+6867GI\n-9394FU\n+9796FU\n-4344FU\n+4857GI\n-5243KI\n+4948KI\n-4445FU\n+7675FU\n-3435FU\n+6776GI\n-9495FU\n+9695FU\n-8586FU\n+8786FU\n-9195KY\n+7687GI\n-0098FU\n+9998KY\n-9598NY\n+8798GI\n-8292HI\n+0097KY\n-9262HI\n+9887GI\n-0076KY\n+0083KA\n-7677KY\n+7877KI\n-0044KE\n+4858KI\n-1415FU\n+1615FU\n-3536FU\n+3736FU\n-4436KE\n+0037FU\n-0028KA\n+1917KY\n-3325KE\n+3736FU\n-2517NK\n+5868KI\n-1115KY\n+0014FU\n-3122GI\n+9792NY\n-6465FU\n+6665FU\n-4546FU\n+4746FU\n-0045FU\n+3848OU\n-4546FU\n+4858OU\n-0082FU\n+8394UM\n-6265HI\n+0048FU\n-0044KY\n+0035KE\n-6545HI\n+0049KY\n-0034FU\n+3543NK\n-5443GI\n+9281NY\n-1727NK\n+0055KE\n-4354GI\n+9461UM\n-3233OU\n+6151UM\n-0065KE\n+7766KI\n-6557NK\n+6857KI\n-0065FU\n+6665KI\n-3324OU\n+6554KI\n-5354FU\n+5142UM\n-0033GI\n+4243UM\n-5455FU\n+1413TO\n-0066FU\n+5766KI\n-2425OU\n+0057KE\n-4647TO\n+4847FU\n-2839UM\n+5867OU\n%TORYO\n")
                       << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2h7h 3c3d 5i4h 8c8d 4h3h 5a4b 7g7f 8d8e 8h7g 2b7g+ 8i7g 4b3b 7h8h 1c1d 1g1f 7a7b 7i6h 4a4b 6g6f 6c6d 6i7h 7b6c 5g5f 6c5d 8h8i 6a5b 3i4h 2a3c 6h6g 9c9d 9g9f 4c4d 4h5g 5b4c 4i4h 4d4e 7f7e 3d3e 6g7f 9d9e 9f9e 8e8f 8g8f 9a9e 7f8g P*9h 9i9h 9e9h+ 8g9h 8b9b L*9g 9b6b 9h8g L*7f B*8c 7f7g 7h7g N*4d 4h5h 1d1e 1f1e 3e3f 3g3f 4d3f P*3g B*2h 1i1g 3c2e 3g3f 2e1g+ 5h6h 1a1e P*1d 3a2b 9g9b+ 6d6e 6f6e 4e4f 4g4f P*4e 3h4h 4e4f 4h5h P*8b 8c9d+ 6b6e P*4h L*4d N*3e 6e4e L*4i P*3d 3e4c+ 5d4c 9b8a 1g2g N*5e 4c5d 9d6a 3b3c 6a5a N*6e 7g6f 6e5g+ 6h5g P*6e 6f6e 3c2d 6e5d 5c5d 5a4b S*3c 4b4c 5d5e 1d1c+ P*6f 5g6f 2d2e N*5g 4f4g+ 4h4g 2h3i+ 5h6g")
                       << QByteArray("1+L7/1p5s1/2p2+Bsp+P/5lp2/2P1pr1kl/1P1GP1P2/1S1KNP1+n1/9/1R3L+bN1 w 2GS7Pgn 124");
    QTest::newRow("2") << QByteArray("V2.2\nN+hoge\nN-foo\nP1-KY-KE-GI-KI-OU-KI-GI-KE-KY\nP2 * -HI *  *  *  *  * -KA * \nP3-FU-FU-FU-FU-FU-FU-FU-FU-FU\nP4 *  *  *  *  *  *  *  *  * \nP5 *  *  *  *  *  *  *  *  * \nP6 *  *  *  *  *  *  *  *  * \nP7+FU+FU+FU+FU+FU+FU+FU+FU+FU\nP8 * +KA *  *  *  *  * +HI * \nP9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n+\n+2726FU\n-8384FU\n+2625FU\n-8485FU\n+6978KI\n-4132KI\n+3938GI\n-7172GI\n+9796FU\n-9394FU\n+5968OU\n-1314FU\n+3736FU\n-8586FU\n+8786FU\n-8286HI\n+2937KE\n-7374FU\n+2524FU\n-2324FU\n+2824HI\n-0023FU\n+2474HI\n-7273GI\n+0087FU\n-8682HI\n+7475HI\n-3334FU\n+7525HI\n-3142GI\n+7776FU\n-4344FU\n+4746FU\n-4243GI\n+4948KI\n-7364GI\n+3847GI\n-4354GI\n+2529HI\n-5465GI\n+7877KI\n-0075FU\n+7675FU\n-6475GI\n+8897KA\n-7564GI\n+6766FU\n-6574GI\n+7978GI\n-2233KA\n+9788KA\n-7485GI\n+6858OU\n-6152KI\n+4756GI\n-5354FU\n+6665FU\n-6475GI\n+0024FU\n-2324FU\n+0025FU\n-5243KI\n+2524FU\n-0022FU\n+5667GI\n-3435FU\n+0076FU\n-7584GI\n+2926HI\n-3536FU\n+2636HI\n-0034FU\n+7766KI\n-3324KA\n+6656KI\n-3435FU\n+3616HI\n-0086FU\n+4645FU\n-8687TO\n+7887GI\n-0086FU\n+8778GI\n-4334KI\n+0083FU\n-8283HI\n+4544FU\n-3536FU\n+1636HI\n-3435KI\n+3635HI\n-2435KA\n+5645KI\n-3513KA\n+4554KI\n-0047FU\n+4838KI\n-5162OU\n+4443TO\n-3243KI\n+5443KI\n-6272OU\n+8997KE\n-0036FU\n+9785KE\n-3637TO\n+0062KI\n-7262OU\n+8844KA\n-6272OU\n+0061GI\n%TORYO\n")
                       << QByteArray("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 2g2f 8c8d 2f2e 8d8e 6i7h 4a3b 3i3h 7a7b 9g9f 9c9d 5i6h 1c1d 3g3f 8e8f 8g8f 8b8f 2i3g 7c7d 2e2d 2c2d 2h2d P*2c 2d7d 7b7c P*8g 8f8b 7d7e 3c3d 7e2e 3a4b 7g7f 4c4d 4g4f 4b4c 4i4h 7c6d 3h4g 4c5d 2e2i 5d6e 7h7g P*7e 7f7e 6d7e 8h9g 7e6d 6g6f 6e7d 7i7h 2b3c 9g8h 7d8e 6h5h 6a5b 4g5f 5c5d 6f6e 6d7e P*2d 2c2d P*2e 5b4c 2e2d P*2b 5f6g 3d3e P*7f 7e8d 2i2f 3e3f 2f3f P*3d 7g6f 3c2d 6f5f 3d3e 3f1f P*8f 4f4e 8f8g+ 7h8g P*8f 8g7h 4c3d P*8c 8b8c 4e4d 3e3f 1f3f 3d3e 3f3e 2d3e 5f4e 3e1c 4e5d P*4g 4h3h 5a6b 4d4c+ 3b4c 5d4c 6b7b 8i9g P*3f 9g8e 3f3g+ G*6b 7b6b 8h4d 6b7b S*6a")
                       << QByteArray("ln1S3nl/2k4p1/1r1p1G2b/ps3B2p/1N1P5/PpP6/3SPp+p1P/2S1K1G2/L7L w G5Prgnp 112");
}


void MaruTest::testFromCsa()
{
    QFETCH(QByteArray, csa);
    QFETCH(QByteArray, expect);
    QFETCH(QByteArray, expectFinalSfen);

    auto result = Sfen::csaToSfen(csa);
    QCOMPARE(result, expect);
    QCOMPARE(Sfen(result).toSfen(), expectFinalSfen);
}



void MaruTest::testGenerateKif_data()
{
    QTest::addColumn<QByteArray>("sfen");
    QTest::addColumn<QByteArrayList>("moves");
    QTest::addColumn<QStringList>("expect");

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
}


void MaruTest::testGenerateKif()
{
    QFETCH(QByteArray, sfen);
    QFETCH(QByteArrayList, moves);
    QFETCH(QStringList, expect);

    Sfen sf(sfen);
    QStringList result = sf.generateKif(moves);
    qDebug() << result.join(" ");
    QCOMPARE(result, expect);
}


void MaruTest::benchmarkSfen()
{
    const QByteArray csa("V2.2\nN+hoge\nN-foo\nP1-KY-KE-GI-KI-OU-KI-GI-KE-KY\nP2 * -HI *  *  *  *  * -KA * \nP3-FU-FU-FU-FU-FU-FU-FU-FU-FU\nP4 *  *  *  *  *  *  *  *  * \nP5 *  *  *  *  *  *  *  *  * \nP6 *  *  *  *  *  *  *  *  * \nP7+FU+FU+FU+FU+FU+FU+FU+FU+FU\nP8 * +KA *  *  *  *  * +HI * \nP9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n+\n+2878HI\n-3334FU\n+5948OU\n-8384FU\n+4838OU\n-5142OU\n+7776FU\n-8485FU\n+8877KA\n-2277UM\n+8977KE\n-4232OU\n+7888HI\n-1314FU\n+1716FU\n-7172GI\n+7968GI\n-4142KI\n+6766FU\n-6364FU\n+6978KI\n-7263GI\n+5756FU\n-6354GI\n+8889HI\n-6152KI\n+3948GI\n-2133KE\n+6867GI\n-9394FU\n+9796FU\n-4344FU\n+4857GI\n-5243KI\n+4948KI\n-4445FU\n+7675FU\n-3435FU\n+6776GI\n-9495FU\n+9695FU\n-8586FU\n+8786FU\n-9195KY\n+7687GI\n-0098FU\n+9998KY\n-9598NY\n+8798GI\n-8292HI\n+0097KY\n-9262HI\n+9887GI\n-0076KY\n+0083KA\n-7677KY\n+7877KI\n-0044KE\n+4858KI\n-1415FU\n+1615FU\n-3536FU\n+3736FU\n-4436KE\n+0037FU\n-0028KA\n+1917KY\n-3325KE\n+3736FU\n-2517NK\n+5868KI\n-1115KY\n+0014FU\n-3122GI\n+9792NY\n-6465FU\n+6665FU\n-4546FU\n+4746FU\n-0045FU\n+3848OU\n-4546FU\n+4858OU\n-0082FU\n+8394UM\n-6265HI\n+0048FU\n-0044KY\n+0035KE\n-6545HI\n+0049KY\n-0034FU\n+3543NK\n-5443GI\n+9281NY\n-1727NK\n+0055KE\n-4354GI\n+9461UM\n-3233OU\n+6151UM\n-0065KE\n+7766KI\n-6557NK\n+6857KI\n-0065FU\n+6665KI\n-3324OU\n+6554KI\n-5354FU\n+5142UM\n-0033GI\n+4243UM\n-5455FU\n+1413TO\n-0066FU\n+5766KI\n-2425OU\n+0057KE\n-4647TO\n+4847FU\n-2839UM\n+5867OU\n%TORYO\n");

    QBENCHMARK {
        Sfen(Sfen::csaToSfen(csa)).toSfen();
    }
}


QTEST_APPLESS_MAIN(MaruTest)
#include "main.moc"
