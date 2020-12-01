#include <iostream>
#include <string_view>
#include <array>
#include <charconv>
#include <bitset>
#include <numeric>
#include <algorithm>

template<typename F>
void split(std::string_view in, char delim, F&& f) {
    std::size_t pos = 0;
    while((pos = in.find(delim)) != std::string_view::npos) {
        f(in.substr(0,pos));
        in.remove_prefix(pos+1);
    }
    f(in);
}

int to_int(std::string_view s) {
    int ret;
    std::from_chars(s.begin(),s.end(),ret);
    return ret;
}

auto parse(std::string_view input) {
    std::vector<int> numbers;
    std::bitset<2020> find;
    split(input,'\n',[&](std::string_view line) {
        int n = to_int(line);
        numbers.push_back(n);
        if(find.test(2020-n)) std::cout << "Part 1: " << n << ' ' << 2020-n << ' ' << n*(2020-n) << '\n';
        find.set(n);
    });
    return std::make_tuple(std::move(numbers),find);
}

void part2(const std::vector<int>& numbers, std::bitset<2020> find) {
    for(auto first = numbers.begin(); *first < 2020/3; ++first) {
        for(auto second = first+1; 2020 - *second - *first > *second; ++second) {
            if(find.test(2020-*second-*first)) {
                std::cout << "Part 2: " << *first << ' ' << *second << ' ' << 2020 - *second - *first << ' ' << *first * *second * (2020 - *first - *second);
                return;
            }
        }
    }
}

void solution(std::string_view input) {
    auto [numbers,find] = parse(input);
    std::sort(numbers.begin(),numbers.end());
    part2(numbers,find);
}

std::string_view input = R"(1895
1602
1592
1714
1403
1766
1654
1771
1957
1533
1661
1761
1711
1869
1541
1595
1819
1735
1894
1316
1777
1450
1526
1888
1968
1877
1972
1988
1655
1369
1636
1453
1969
1239
1717
1444
1907
1682
1358
1706
1482
1852
1689
1905
1262
1956
770
1507
1314
1890
784
1710
1418
597
1417
1587
2003
1889
879
1534
279
1302
1976
1936
1590
1939
1489
1966
1238
1481
1412
1675
2001
1543
1220
1352
1536
1879
1892
2006
1642
1321
1600
1908
2009
1784
1260
1881
1897
1933
1048
1851
2005
1626
1441
1945
1742
1734
1816
1919
1802
1460
1845
1914
1652
1943
1521
1830
1477
1866
1702
1629
1932
1671
1707
1577
1962
1518
1989
1502
61
1546
1264
1651
2000
1443
1931
1882
1583
1597
1487
1255
1779
1782
1540
1580
1294
1691
1337
1743
1632
1348
2010
1794
1876
1808
1647
422
1994
1864
1996
1738
1998
1749
1789
1395
1997
1440
1676
1527
1523
1836
1366
1700
1826
1426
1709
166
1958
1909
1428
1984
521
1760
156
1296
1475
1566
1573
1696
1471
1788
1809
1942
1461
1559
1699
1504
1465
1658
1973
1679
1376
1985
1503
1517
1825
1847
1528
1246)";

int main() {
    solution(input);
}
