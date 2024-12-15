---
title: Spetification of syamfp
auther: KATOI
date: 2024-11-25
lang: ja-JP
---

# SYAMFP

## 目次 <!-- omit in toc -->

- [1. 概要](#1-概要)
- [2. 機能詳細](#2-機能詳細)
	- [2.1. 対応演算子](#21-対応演算子)
	- [2.2. 対応関数](#22-対応関数)
	- [2.3. 対応定数](#23-対応定数)
	- [2.4. 使用方法(サンプル)](#24-使用方法サンプル)

## 1. 概要

**S**hunting **Y**ard **A**lgorithm を利用した、数式 (**M**athematical **F**ormula) のパーサー (**P**arser) です。

## 2. 機能詳細

### 2.1. 対応演算子

| 文字列 | 演算子 | 備考                                               |
| :----: | :----: | :------------------------------------------------- |
|   +    |   +    | 足し算                                             |
|   -    |   -    | 引き算                                             |
|   *    |   ×    | 掛け算                                             |
|   /    |   ÷    | 割り算                                             |
|   ^    |  pow   | 冪乗                                               |
|   (    |   (    | 左括弧                                             |
|   )    |   )    | 右括弧                                             |
|   ,    |   ,    | コンマ(括弧内の引数を区別する. pow(2,3)=2^3 など.) |

### 2.2. 対応関数

| 文字列 |  関数  | 備考            |
| :----: | :----: | :-------------- |
|  sin   |  sin   |                 |
|  cos   |  cos   |                 |
|  tan   |  tan   |                 |
|  asin  |  asin  |                 |
|  acos  |  acos  |                 |
|  atan  |  atan  |                 |
|  sinh  |  sinh  |                 |
|  cosh  |  cosh  |                 |
|  tanh  |  tanh  |                 |
| asinh  | asinh  |                 |
| acosh  | acosh  |                 |
| atanh  | atanh  |                 |
|  exp   |  exp   |                 |
|  log   | log_e  | 自然対数        |
|   ln   | log_e  | 自然対数        |
| log10  | log_10 | 常用対数        |
|  sqrt  |  sqrt  | 平方根          |
|  pow   |  pow   | ^ と同値        |

### 2.3. 対応定数
|   文字列   |       定数        | 備考                        |
| :--------: | :---------------: | :-------------------------- |
|     pi     |     \( \pi \)     |                             |
|     e      |      \( e \)      | 変数として e は使用できない |
|   sqrt2    |   \( \sqrt2 \)    |                             |
|    ln2     |  \( \log_e 2 \)   |                             |
|    ln10    |  \( \log_e 10 \)  |                             |
|   log2e    |  \( \log_2 e \)   |                             |
|   log10e   | \( \log_{10} e \) |                             |
|   inv_pi   |    \( 1/\pi \)    |                             |
| inv_sqrtpi | \( 1/\sqrt\pi \)  |                             |
|   egamma   |   \( \gamma \)    | オイラー定数                |
|    phi     |    \( \phi \)     | 黄金律                      |

### 2.4. 使用方法(サンプル)

``` C++
#include "syamfp.hpp"
#include <iostream>

int main()
{
	SYAMFP::VariableTable table;
	table.add("a", 2);

	SYAMFP::Parser parser;
	parser.parse("a*x^3 - 2", table);

	auto func = parser.ret_func()
	std::cout << "f(x=3)=" << func(3) << std::endl; // return 2 * 3^3 - 2 = 52

	return 0;
}
```
