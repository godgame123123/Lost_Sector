// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopUtilityLibrary.h"
#include "Algo/Sort.h"

void UShopUtilityLibrary::SortItemsByPrice(TArray<UItemDataBase*>& Items)
{
	
	Algo::Sort(Items, [](const UItemDataBase* A, const UItemDataBase* B)
		{
			// 데이터가 유효한지 체크
			if (!IsValid(A) || !IsValid(B)) return false;

			// 오름차순 정렬 (A가 B보다 싸면 앞으로)
			// 만약 내림차순(비싼 순)으로 하고 싶으면 < 를 > 로 바꾸세요.
			return A->Value < B->Value;
		});
}
