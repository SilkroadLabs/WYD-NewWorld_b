
#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <io.h>
#include <errno.h>
#include "..\Basedef.h"
#include "..\CPSock.h"
#include "..\ItemEffect.h"
#include "Language.h"
#include "CItem.h"
#include "Server.h"
#include "ProcessClientMessage.h"
#include "GetFunc.h"
#include "SendFunc.h"
#include "Functions.h"
#include <iostream>  
#include "wMySQL.h"


/*Funções Sistema leilao*/
void goResetLeilao(int clientId)
{
	if (clientId <= MOB_EMPTY || clientId >= MAX_USER || pUser[clientId].Mode != USER_PLAY)
		return;

	pMob[clientId].IsAutoTrading = false;
	memset(&pMob[clientId].AutoTrade, 0, sizeof(pMob[clientId].AutoTrade));
	RemoveMobLeilaoStore(clientId);
	pUser[clientId].sxCountSerch = 0; 

}

void RemoveMobLeilaoStore(int clientId)
{
	bool isPlayer = (clientId > 0 && clientId < 1000);

	if (isPlayer)
	{
		if ((pUser[clientId].Mode != USER_PLAY))
			return;
	}


	INT32 lenName = strlen(pMob[clientId].MOB.MobName);

	for (INT32 i = 1000; i < MAX_MOB; i++)
	{
		if (pMob[i].Mode == 0)
			continue;

		if (!pMob[i].IsAutoTrading)
			continue;

		if (!strncmp(pMob[i].MOB.MobName, pMob[clientId].MOB.MobName, lenName))
		{
			memset(&pMob[i].AutoTrade, 0, sizeof(pMob[0].AutoTrade));
			memset(&pMob[i].IsAutoTrading, 0, sizeof(pMob[0].IsAutoTrading));
			DeleteMob(i, 3);
			break;
		}
	}

	return;
}

INT32 CreateMobLeilaoStore(int posX, int posY, int seller)
{
	INT32 LOCAL_1 = GetEmptyNPCMob();
	if (LOCAL_1 == 0)
		return -1;

	pMob[LOCAL_1] = CMob{};
	strncpy_s(pMob[LOCAL_1].MOB.MobName, "Base", 16);
	memset(&pMob[LOCAL_1].PartyList, 0, sizeof(INT16) * 12);

	INT32 LOCAL_2 = ReadMob(&pMob[LOCAL_1].MOB, "npc");
	if (LOCAL_2 == 0)
		return false;

	strncpy_s(pMob[LOCAL_1].MOB.MobName, pMob[seller].MOB.MobName, 16);
	pMob[LOCAL_1].MOB.MobName[15] = 0;

	for (INT32 LOCAL_3 = 0; LOCAL_3 < 16; LOCAL_3++)
	{
		if (pMob[LOCAL_1].MOB.MobName[LOCAL_3] == '_')
			pMob[LOCAL_1].MOB.MobName[LOCAL_3] = ' ';
		if (pMob[LOCAL_1].MOB.MobName[LOCAL_3] == '@')
			pMob[LOCAL_1].MOB.MobName[LOCAL_3] = ' ';
	}

	memset(&pMob[LOCAL_1].Affect, 0, sizeof(STRUCT_AFFECT) * 32);

	// Trava para não andar: inicializar a posição e manter fixa
	for (INT32 LOCAL_3 = 0; LOCAL_3 < 5; LOCAL_3++)
	{
		pMob[LOCAL_1].SegmentListX[LOCAL_3] = posX;
		pMob[LOCAL_1].SegmentListY[LOCAL_3] = posY;
	}

	strncpy_s(pMob[LOCAL_1].Tab, "", 16);

	pMob[LOCAL_1].MOB.Equip[0].sIndex = 230;
	pMob[LOCAL_1].MOB.Equip[6].sIndex = 0;
	pMob[LOCAL_1].MOB.Equip[7].sIndex = 0;
	pMob[LOCAL_1].MOB.Equip[14].sIndex = 0;
	pMob[LOCAL_1].MOB.CurrentScore.Con = 400;

	pMob[LOCAL_1].GenerateIndex = -1;
	pMob[LOCAL_1].Formation = 0;
	pMob[LOCAL_1].RouteType = 0;
	pMob[LOCAL_1].Mode = 4; // Modo específico para NPC de comércio
	pMob[LOCAL_1].SegmentProgress = 0;
	pMob[LOCAL_1].Leader = 0;
	pMob[LOCAL_1].WaitSec = 10;


	pMob[LOCAL_1].GetCurrentScore(MAX_USER);

	pMob[LOCAL_1].MOB.CurrentScore.Hp = pMob[LOCAL_1].MOB.CurrentScore.MaxHp;
	pMob[LOCAL_1].SegmentDirection = 0;

	memset(&pMob[LOCAL_1].EnemyList, 0, 8);

	int LOCAL_5 = pMob[LOCAL_1].SegmentListX[0];
	int LOCAL_6 = pMob[LOCAL_1].SegmentListY[0];

	INT32 LOCAL_7 = GetEmptyMobGrid(LOCAL_1, &LOCAL_5, &LOCAL_6);
	if (LOCAL_7 == 0)
	{
		pMob[LOCAL_1].Mode = 0;
		pMob[LOCAL_1].MOB.MobName[0] = 0;
		pMob[LOCAL_1].GenerateIndex = -1;
		return -2;
	}

	pMob[LOCAL_1].LastTime = clock();

	pMob[LOCAL_1].SegmentX = LOCAL_5;
	pMob[LOCAL_1].TargetX = LOCAL_5;
	pMob[LOCAL_1].LastX = LOCAL_5;

	pMob[LOCAL_1].SegmentY = LOCAL_6;
	pMob[LOCAL_1].TargetY = LOCAL_6;
	pMob[LOCAL_1].LastY = LOCAL_6;

	INT32 LOCAL_17 = pMob[LOCAL_1].MOB.BaseScore.MaxHp;
	if (LOCAL_17 != 0)
	{
		SetAffect(LOCAL_1, LOCAL_17, 30000, 200);
		SetTick(LOCAL_1, LOCAL_17, 30000, 200);
	}

	pMobGrid[LOCAL_6][LOCAL_5] = LOCAL_1;


	pMob[LOCAL_1].IsAutoTrading = true;

	SendGridMob(seller);
	SendGridMob(LOCAL_1);

	// Certifique-se de que o modo está correto para um NPC de comércio
	pMob[LOCAL_1].Mode = 1; // Substitua por um valor de modo adequado

	return LOCAL_1;
}
 
float TimeRemaining(int dia, int mes, int ano)
{
	time_t rawnow = time(NULL);
	struct tm now; localtime_s(&now, &rawnow);

	int month = now.tm_mon; //0 Janeiro, 1 Fev
	int day = now.tm_mday;
	int year = now.tm_year;

	struct std::tm a = { 0,0,0, day, month, year };
	struct std::tm b = { 0,0,0, dia, mes - 1, ano - 1900 };

	std::time_t x = std::mktime(&a);
	std::time_t y = std::mktime(&b);

	if (x != (std::time_t)(-1) && y != (std::time_t)(-1))
	{
		double difference = (std::difftime(y, x) / (60 * 60 * 24));
		return static_cast<float>(difference);
	}

	return 0;
}

INT32 GetFirstSlot(int clientId, int itemId)
{
	STRUCT_ITEM* item = pMob[clientId].MOB.Carry;
	for (int i = 0; i < 30; i++)
	{
		if (item[i].sIndex == itemId)
			return i;
	}

	if (item[60].sIndex == 3467)
	{
		float remainig = TimeRemaining(item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);
		if (remainig > 0.0f)
		{
			for (int i = 30; i < 45; i++)
			{
				if (item[i].sIndex == itemId)
					return i;
			}
		}
		else
		{
			printf("\nBolsa do Andarilho acabou. Slot 60. %02d/%02d/%04d", item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);
			printf("\nBolsa do andarilho acabou. Data de finalizaaao: %02d/%02d/%04d", item[60].EFV1, item[60].EFV2, item[60].EFV3 + 1900);

			memset(&item[60], 0, sizeof STRUCT_ITEM);

			// Atualiza o inventario
			SendItem(clientId, ITEM_PLACE_CARRY, 60, &item[60]);
		}
	}

	if (item[61].sIndex == 3467)
	{
		float remainig = TimeRemaining(item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);
		if (remainig > 0.0f)
		{
			for (int i = 45; i < 60; i++)
			{
				if (item[i].sIndex == itemId)
					return i;
			}
		}
		else
		{
			printf("\nBolsa do Andarilho acabou. Slot 61. %02d/%02d/%04d", item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);
			printf("\nBolsa do andarilho acabou. Data de finalizaaao: %02d/%02d/%04d", item[61].EFV1, item[61].EFV2, item[61].EFV3 + 1900);
			memset(&item[61], 0, sizeof STRUCT_ITEM);

			// Atualiza o inventario
			SendItem(clientId, ITEM_PLACE_CARRY, 61, &item[61]);
		}
	}

	return -1;
}

INT32 GetItemAmount(STRUCT_ITEM* item)
{
	INT32 amount = 1;

	for (INT8 i = 0; i < 3; i++)
	{
		if (item->stEffect[i].cEffect == EF_AMOUNT)
		{
			amount = item->stEffect[i].cValue;
			if (amount == 0)
				amount = 1;

			break;
		}
	}

	return amount;
}

void AmountMinusPlus(STRUCT_ITEM* item, int qtd)
{
	int index = 0;
	int amount = 0;
	for (int i = 0; i < 3; i++)
	{
		if (item->stEffect[i].cEffect == EF_AMOUNT)
		{
			index = i;
			amount = item->stEffect[i].cValue;

			break;
		}
	}

	if (amount <= 1)
		memset(item, 0, sizeof STRUCT_ITEM);
	else
	{
		if (item->stEffect[index].cValue == qtd)
			memset(item, 0, sizeof STRUCT_ITEM);
		else
		{
			if (item->stEffect[index].cValue && item->stEffect[index].cValue < 120)
			{
				item->stEffect[index].cValue = item->stEffect[index].cValue - qtd;
			}
		}
	}
}
 
void SendLeilaoStoreInfos(int clientId, int targetItemType) {
	if (clientId <= MOB_EMPTY || clientId >= MAX_USER || pUser[clientId].Mode != USER_PLAY)
		return;

	MSG_LeilaoStore auctionPage;
	memset(&auctionPage, 0, sizeof(auctionPage));
	auctionPage.Size = sizeof(MSG_LeilaoStore);
	auctionPage.ID = clientId;
	auctionPage.Type = 0x397B;

	int itemCount = 0;
	int pageIndex = 0;

	for (int i = 0; i < MAX_SLOT_LEILAO_AUTOTRADE; ++i) {
		// Verifica se o slot tem um item válido
		if (pMob[clientId].AutoTrade.List[i].Item.sIndex <= 0)
		{
			continue;
		}

		// Filtra itens por tipo, se necessário
		if (targetItemType != 0 && pMob[clientId].AutoTrade.List[i].Categoria != targetItemType)
			continue;

		// Cópia dos dados para a estrutura de envio
		memcpy(&auctionPage.List[itemCount], &pMob[clientId].AutoTrade.List[i], sizeof(LeilaoStoreAux));
		itemCount++;

		// Verifica se é necessário enviar uma nova página
		if (itemCount >= MAX_SLOT_LEILAO_AUTOTRADE) {
			auctionPage.TotalPages = ++pageIndex;
			auctionPage.CurrentPage = pageIndex;
			if (!pUser[clientId].cSock.AddMessage((char*)&auctionPage, sizeof(MSG_LeilaoStore)))
				CloseUser(clientId);

			memset(&auctionPage, 0, sizeof(auctionPage));
			auctionPage.Size = sizeof(MSG_LeilaoStore);
			auctionPage.ID = clientId;
			auctionPage.Type = 0x397B;
			itemCount = 0;
		}
	}

	// Enviar a última página, se houver itens
	if (itemCount > 0) {
		auctionPage.TotalPages = ++pageIndex;
		auctionPage.CurrentPage = pageIndex;
		if (!pUser[clientId].cSock.AddMessage((char*)&auctionPage, sizeof(MSG_LeilaoStore)))
			CloseUser(clientId);
	}
}

void SendLeilaoStoreMyInfos(int clientId)
{
	if (clientId <= MOB_EMPTY || clientId >= MAX_USER || pUser[clientId].Mode != USER_PLAY)
		return;
	 

	int itemCount = 0;
	MSG_LeilaoStore auctionPage;
	memset(&auctionPage, 0, sizeof(auctionPage));
	auctionPage.Size = sizeof(MSG_LeilaoStore);
	auctionPage.Type = 0x397B;
	auctionPage.ID = clientId;
	auctionPage.TotalPages = 1;
	auctionPage.CurrentPage = 1;


	for (int i = 0; i < MAX_SLOT_LEILAO_AUTOTRADE; i++)
	{
		if (itemCount >= MAX_SLOT_LEILAO_AUTOTRADE)
			break;

		auctionPage.List[itemCount].OwnerId = clientId;
		strncpy_s(auctionPage.List[itemCount].Name, pMob[clientId].AutoTrade.List[i].Name, sizeof(auctionPage.List[itemCount].Name) - 1);
		auctionPage.List[itemCount].TaxRate = pMob[clientId].AutoTrade.List[i].TaxRate;
		auctionPage.List[itemCount].Item = pMob[clientId].AutoTrade.List[i].Item;
		auctionPage.List[itemCount].Price = pMob[clientId].AutoTrade.List[i].Price;
		auctionPage.List[itemCount].SlotBank = pMob[clientId].AutoTrade.List[i].SlotBank;
		auctionPage.List[itemCount].Categoria = pMob[clientId].AutoTrade.List[i].Categoria;

		itemCount++;
	}

	auctionPage.Size = sizeof(MSG_LeilaoStore) + itemCount * sizeof(LeilaoStoreAux);

	if (!pUser[clientId].cSock.AddMessage((char*)&auctionPage, auctionPage.Size))
		CloseUser(clientId);
	 
}

void SendLeilaoStoreAlvoInfos(int clientId, int Dono) {
	if (clientId <= MOB_EMPTY || clientId >= MAX_USER || pUser[clientId].Mode != USER_PLAY)
		return;

	int itemCount = 0;
	MSG_LeilaoStore auctionPage;
	memset(&auctionPage, 0, sizeof(auctionPage));
	auctionPage.Size = sizeof(MSG_LeilaoStore);
	auctionPage.Type = 0x397B;
	auctionPage.ID = clientId;
	auctionPage.TotalPages = 1;
	auctionPage.CurrentPage = 1;


	for (int i = 0; i < MAX_SLOT_LEILAO_AUTOTRADE; i++)
	{
		if (itemCount >= MAX_SLOT_LEILAO_AUTOTRADE)
			break;

		auctionPage.List[itemCount].OwnerId = Dono;
		strncpy_s(auctionPage.List[itemCount].Name, pMob[Dono].AutoTrade.List[i].Name, sizeof(auctionPage.List[itemCount].Name) - 1);
		auctionPage.List[itemCount].TaxRate = pMob[Dono].AutoTrade.List[i].TaxRate;
		auctionPage.List[itemCount].Item = pMob[Dono].AutoTrade.List[i].Item;
		auctionPage.List[itemCount].Price = pMob[Dono].AutoTrade.List[i].Price;
		auctionPage.List[itemCount].SlotBank = pMob[Dono].AutoTrade.List[i].SlotBank;
		auctionPage.List[itemCount].Categoria = pMob[Dono].AutoTrade.List[i].Categoria;

		itemCount++;
	}

	auctionPage.Size = sizeof(MSG_LeilaoStore) + itemCount * sizeof(LeilaoStoreAux);

	if (!pUser[clientId].cSock.AddMessage((char*)&auctionPage, auctionPage.Size))
		CloseUser(clientId);
}
 
void SendLeilaoStoreAllInfos(int clientId, int type, int requestedPage)
{
	if (clientId <= MOB_EMPTY || clientId >= MAX_USER || pUser[clientId].Mode != USER_PLAY)
		return;

	std::vector<LeilaoStoreAux> allItems;

	// Coletando os itens de todos os jogadores que estão em AutoTrade
	for (int player = 0; player < MAX_USER; ++player)
	{
		if (pUser[player].Mode != USER_PLAY)
			continue;

		if (!pMob[player].IsAutoTrading)
			continue;

		for (int i = 0; i < MAX_SLOT_LEILAO_AUTOTRADE; i++)
		{
			if (pMob[player].AutoTrade.List[i].Item.sIndex == 0)
				continue;

			if (type != 0 && type != pMob[player].AutoTrade.List[i].Categoria)
				continue;

			LeilaoStoreAux itemInfo;
			itemInfo.OwnerId = player;

			// Copia as informações do item sem alterar sua posição original
			strncpy_s(itemInfo.Name, pMob[player].AutoTrade.List[i].Name, sizeof(itemInfo.Name) - 1);
			itemInfo.TaxRate = pMob[player].AutoTrade.List[i].TaxRate;
			itemInfo.Item = pMob[player].AutoTrade.List[i].Item;
			itemInfo.Price = pMob[player].AutoTrade.List[i].Price;
			itemInfo.SlotBank = pMob[player].AutoTrade.List[i].SlotBank;
			itemInfo.Categoria = pMob[player].AutoTrade.List[i].Categoria;

			allItems.push_back(itemInfo);
		}
	}

	// Calculando o total de itens e páginas
	int totalItems = allItems.size();
	int itemsPerPage = MAX_SLOT_LEILAO_AUTOTRADE;  // Quantidade de itens por página
	int totalPages = (totalItems + itemsPerPage - 1) / itemsPerPage;

	// Verificando se a página solicitada é válida
	if (requestedPage < 1 || requestedPage > totalPages)
		requestedPage = 1;  // Caso a página solicitada seja inválida, voltamos para a página 1

	// Preparando o pacote para a página solicitada
	MSG_LeilaoStore auctionPage;
	memset(&auctionPage, 0, sizeof(auctionPage));
	auctionPage.Size = sizeof(MSG_LeilaoStore);
	auctionPage.Type = 0x397B;
	auctionPage.ID = clientId;
	auctionPage.TotalPages = totalPages;
	auctionPage.CurrentPage = requestedPage;

	int startIdx = (requestedPage - 1) * itemsPerPage;
	int endIdx = min(startIdx + itemsPerPage, totalItems);
	int itemCount = 0;

	for (int i = startIdx; i < endIdx; ++i)
	{
		auctionPage.List[itemCount++] = allItems[i];
	}

	auctionPage.Size = sizeof(MSG_LeilaoStore) + itemCount * sizeof(LeilaoStoreAux);

	// Enviando os dados para o cliente
	if (!pUser[clientId].cSock.AddMessage((char*)&auctionPage, auctionPage.Size))
	{
		CloseUser(clientId);
		return;
	}
}

/*Recupera informações*/
void Exec_MSG_LeilaoStore(int clientId, char* pMsg)
{ 
	MSG_GetLeilaoStores* p = (MSG_GetLeilaoStores*)pMsg;

	if (p->Size > sizeof(MSG_GetLeilaoStores))
	{
		printf("Pacote: 0x%X - Tamanho: %d - Index: %d\n",p->Type, p->Size, p->ID);
		return;
	} 

	if (clientId <= 0 || clientId >= MAX_USER)
	{
		AddCrackError(clientId, 10, 803);
		return;
	}

	if (pUser[clientId].Mode != USER_PLAY)
	{
		AddCrackError(clientId, 10, 804);
		return;
	}

	//// Verifica se o tempo de atraso permite a nova busca
	if (pUser[clientId].sxCountSerch != 0)
	{
		int isTime = GetTickCount64() - pUser[clientId].sxCountSerch; // ATRASO PACOTE DE LOGIN

		if (isTime < 500)
		{ 
			return;
		} 
	}

	// Atualiza o tempo de atraso para evitar spam
	pUser[clientId].sxCountSerch = GetTickCount64();

	if (pMob[clientId].MOB.CurrentScore.Hp == 0)
	{
		AddCrackError(clientId, 10, 805);
		return;
	}

	// Define o tipo de item com base no comando
	int tipo = p->itemType;

	if (tipo == -1)//somente minhas informações
	{    
		if (!pMob[clientId].IsAutoTrading)
		{ 
			return;
		}  
		 SendLeilaoStoreMyInfos(clientId);  
	} 
	else
	{ 
		SendLeilaoStoreAllInfos(clientId, tipo, p->currentpage); 
	}

	UpdateLeilaoSaldo(clientId);
}

/*Cria Mob e da permissao para anunciar itens*/
void Exec_MSG_CreateMobLeilao(int clientId, char* pMsg)
{
	std::cout << "Informações dos itens da loja\n";
	MSG_LeilaoStore* p = (MSG_LeilaoStore*)(pMsg);

	if (p->Size > sizeof(MSG_LeilaoStore))
	{
		printf("Pacote: 0x%X - Tamanho: %d - Index: %d\n", p->Type, p->Size, p->ID);
		return;
	}


	// Verificar se o usuário já tem uma loja ativa
	for (int i = 1; i < MAX_USER; i++)
	{
	    auto& user = pUser[i];
	    if (user.Mode != USER_PLAY || !pMob[i].IsAutoTrading)
	        continue;

	    // Comparar endereços MACCheckLojaTime
	    if (memcmp(pUser[clientId].MacAddress, user.MacAddress, 6) == 0)
	    {
	        // Se já tiver uma loja ativa com o mesmo endereço MAC, retornar falso
	        SendClientMessage(clientId, "Você tem loja ativa!!!");
	      //  return;
	    }
	}


	goResetLeilao(clientId);

	if (!pMob[clientId].MOB.CurrentScore.Hp)
	{ 
		AddCrackError(clientId, 10, 800);
		return;
	}
	if (pUser[clientId ].Mode != USER_PLAY)
	{
		SendHpMode(clientId);
		AddCrackError(clientId, 10, 801);
		RemoveTrade(clientId);
		return;
	}

	if (pUser[clientId].Trade.OpponentID)
	{
		RemoveTrade(clientId);
		return;
	}

	RemoveTrade(clientId);

	INT32 posX = pMob[clientId].TargetX;
	INT32 posY = pMob[clientId].TargetY;
	INT32 villageId = BASE_GetVillage(posX, posY);
	if (villageId < 0 || villageId >= 5)
	{
		SendClientMessage(clientId, g_pMessageStringTable[_NN_OnlyVillage]);
		return;
	}

	if (clientId <= 0 || clientId >= MAX_USER)
		return;



	int mobid = CreateMobLeilaoStore(posX, posY, clientId);
	if (mobid <= 0)
	{
		SendClientMessage(clientId, "Não foi possivel abrir a loja.");
		return;
	}

	const char* tiposDeVenda[] = { "Desconhecido", "Donate", "Gold", "Bis" };
	 

	for (int i = 0; i < MAX_SLOT_LEILAO_AUTOTRADE; i++)
	{
		p->List[i].Name[23] = '\0';
		p->List[i].Name[22] = '\0';

		int tipoIndex = (pMob[clientId].AutoTrade.List[i].Categoria >= 1 && pMob[clientId].AutoTrade.List[i].Categoria <= 3) ? pMob[clientId].AutoTrade.List[i].Categoria : 0;
		int itemIndice = pMob[clientId].AutoTrade.List[i].Item.sIndex;

		memcpy(&pMob[clientId].AutoTrade.List[i].Name, &p->List[i].Name, 24);
		pMob[clientId].IsAutoTrading = true;
		pMob[mobid].IsAutoTrading = true;
		pMob[clientId].AutoTrade.List[i].OwnerId = clientId;

		std::cout << "Informações dos itens da loja\n";
		std::cout << "OwnerId da loja: " << pMob[clientId].AutoTrade.List[i].OwnerId << "\n";
		std::cout << "Nome da loja: " << pMob[clientId].AutoTrade.List[i].Name << "\n";
		std::cout << "Posição criada: " << pMob[clientId].TargetX << "x " << pMob[clientId].TargetY << "\n";
		std::cout << "[ITENS]" << "\n";
		std::cout << "SlotBank do Item da Loja: " << (int)pMob[clientId].AutoTrade.List[i].SlotBank << "\n";
		std::cout << "Tipo de Venda: " << tiposDeVenda[tipoIndex] << "\n";
		std::cout << "Slot [" << std::to_string(pMob[clientId].AutoTrade.List[i].Categoria) << "] - " << g_pItemList[itemIndice].Name << " " << ". Preço de " << pMob[clientId].AutoTrade.List[i].Price << "\n";
	}

	memcpy(&pMob[mobid].AutoTrade, &pMob[clientId].AutoTrade, sizeof pMob[clientId].AutoTrade);
	pMob[mobid].IsAutoTrading = true; 

	MSG_CreateMobTrade sm_tb;
	memset(&sm_tb, 0, sizeof(MSG_CreateMobTrade));
	GetCreateMobTrade(mobid, &sm_tb); 
	GridMulticast(pMob[clientId].TargetX, pMob[clientId].TargetY, (MSG_STANDARD*)&sm_tb, 0);
	 
	SendClientSignalParm(clientId, 0x7530, pLeilaoStatus, 1);
	SendLeilaoStoreMyInfos(clientId);
	SendClientMessage(clientId, "Sua lojinha foi criada com sucesso."); 
	Log(temp, "-system", 0);
	return;

}

/*Abre Mob e envia informações itens*/
void Exec_MSG_OpenMobLeilao(int clientId, char* pMsg)
{ 
	MSG_STANDARDPARM* p = (MSG_STANDARDPARM*)(pMsg);

	INT32 idx = p->Parm;
	INT32 Dono = 0;

	if (pMob[clientId].MOB.CurrentScore.Hp == 0 || pUser[clientId].Mode != USER_PLAY)
	{
		AddCrackError(clientId, 10, 804);
		return ;
	}

	if (pMob[idx].MOB.CurrentScore.Hp == 0)
	{
		AddCrackError(clientId, 10, 805);
		return;
	}

	if (!pMob[idx].IsAutoTrading)
		return;

	Dono = pMob[idx].AutoTrade.List->OwnerId;


	if (Dono <= 0 || Dono >= MAX_USER)
	{
		AddCrackError(clientId, 10, 803);
		return;
	}

	if (pUser[Dono].Mode != USER_PLAY)
	{
		AddCrackError(clientId, 10, 804);
		return;
	}

	SendLeilaoStoreAlvoInfos(clientId, Dono);
	return;

}

/*Cria anuncio de itens no Sistema de leilão*/
void Exec_MSG_AddItemLeilaoStore(int clientId, char* pMsg)
{ 
	pAddItemLeilaoStore* p = (pAddItemLeilaoStore*)(pMsg);

	if (p->Size > sizeof(pAddItemLeilaoStore))
	{
		printf("Pacote: 0x%X - Tamanho: %d - Index: %d\n", p->Type, p->Size, p->ID);
		return;
	} 

	int itemType = p->itemType;
	int SlotBank = p->SlotBank;
	int itemPrice = p->itemPrice;
	STRUCT_ITEM newItem = p->item;

	bool itemAdded = false;

	// Log inicial: recepção do pacote
	// printf("\n Recebido pacote 0x397C - Tentativa de adicionar item ao AutoTrade. Tipo: %d, SlotBank: %d, Preço: %d", itemType, SlotBank, itemPrice);
	INT32 itemId = p->item.sIndex;
	INT32 i = 0;
	char strSexy[128]{};

	int sTepPacket = 0;

	if (itemId == 0)
		return;

	if (itemPrice < 0 || itemPrice > 1999999999)
		return;


	if (itemType == 3 && itemPrice > 100)//BIs
	{
		SendClientMessage(clientId, "Limite de 100 bi para cada 1 item!");
		SendLeilaoStoreInfos(clientId, 0);
		return;
	}


	if (itemId == 508 || itemId == 509 || itemId == 522 || (itemId >= 526 && itemId <= 531) || itemId == 446
		|| itemId == 3993 || itemId == 3994)
		return;

	if (BASE_GetItemAbility(&p->item, EF_NOTRADE))
	{
		SendClientMessage(clientId, "Este item não é negociavel.");
		SendLeilaoStoreInfos(clientId, 0);

		SendItem(clientId, ITEM_PLACE_CARGO, SlotBank, &pUser[clientId].Cargo[SlotBank]);
		return;
	}

	if (!pMob[clientId].IsAutoTrading)
	{
		SendClientMessage(clientId, "Você precisa criar uma lojinha para anunciar itens!");
		SendLeilaoStoreInfos(clientId, 0);

		SendItem(clientId, ITEM_PLACE_CARGO, SlotBank, &pUser[clientId].Cargo[SlotBank]);
		return;
	}

	if (SlotBank < 0 || SlotBank >= 168)
		return;

	INT32 verify = memcmp(&pUser[clientId].Cargo[SlotBank], &p->item, 8);

	if (verify != 0)
	{
		return;
	}

	if (!itemType)
	{
		snprintf(strSexy, sizeof strSexy, "%s esta sem definição de valor. Processo reiniciado!", g_pItemList[itemId].Name);
		SendClientMessage(clientId, strSexy);
		SendLeilaoStoreInfos(clientId, 0);

		SendItem(clientId, ITEM_PLACE_CARGO, SlotBank, &pUser[clientId].Cargo[SlotBank]);
		return;
	}


	p->Name[23] = '\0';
	p->Name[22] = '\0';

	INT32 posX = pMob[clientId].TargetX;
	INT32 posY = pMob[clientId].TargetY;
	INT32 villageId = BASE_GetVillage(posX, posY);

	if (villageId < 0 || villageId >= 5)
	{
		SendClientMessage(clientId, g_pMessageStringTable[_NN_OnlyVillage]);
		SendLeilaoStoreInfos(clientId, 0);

		SendItem(clientId, ITEM_PLACE_CARGO, SlotBank, &pUser[clientId].Cargo[SlotBank]);
		return;
	}

	int CityTax = g_pGuildZone[villageId].CityTax;

	for (int i = 0; i < MAX_SLOT_LEILAO_AUTOTRADE; i++)
	{
		if (pMob[clientId].AutoTrade.List[i].Item.sIndex == 0)
		{
			// Vaga disponível encontrada, adicionar o item
			strncpy_s(pMob[clientId].AutoTrade.List[i].Name, pMob[clientId].AutoTrade.List[i].Name, sizeof(pMob[clientId].AutoTrade.List[i].Name) - 1);
			pMob[clientId].AutoTrade.List[i].OwnerId = clientId;
			pMob[clientId].AutoTrade.List[i].TaxRate = CityTax;
			pMob[clientId].AutoTrade.List[i].Item = newItem;
			pMob[clientId].AutoTrade.List[i].Categoria = itemType;
			pMob[clientId].AutoTrade.List[i].SlotBank = SlotBank;
			pMob[clientId].AutoTrade.List[i].Price = itemPrice;
			itemAdded = true;

			// Log de sucesso: item adicionado
		printf("\nItem adicionado ao AutoTrade. Tipo: %d, SlotBank: %d, Preço: %d, Posição no AutoTrade: %d", itemType, SlotBank, itemPrice, i);
			break;
		}
	}

	if (itemAdded)
	{

		// Declara uma variável para armazenar o tipo de item como string
		const char* itemTypeName = "";

		// Verifica o tipo de item e define a variável itemTypeName apropriada
		switch (itemType)
		{
		case 1: // donate
			itemTypeName = "donate";
			break;
		case 2: // gold
			itemTypeName = "gold";
			break;
		case 3: // barras de 1b
			itemTypeName = "barras de 1b";
			break;
		default:
			itemTypeName = "NA";
			break;
		}

		snprintf(strSexy, sizeof(strSexy), "Item '%s' anunciado por %d em %s.", g_pItemList[itemId].Name, itemPrice, itemTypeName);


		// Envia a mensagem para o cliente e atualiza a loja de leilão
		SendClientMessage(clientId, strSexy);
		SendLeilaoStoreInfos(clientId, 0);
	}
	else
	{
		// Log de falha: não há vagas disponíveis
		printf("\nFalha ao adicionar item ao AutoTrade: Não há vagas disponíveis. Tipo: %d, SlotBank: %d, Preço: %d", itemType, SlotBank, itemPrice);

		// Enviar uma mensagem ao cliente indicando que não há vagas disponíveis
		SendClientMessage(clientId, "Não há vagas disponíveis no AutoTrade.");
	}
	return;
}

/*Negociação de itens no Sistema de leilão*/
void Exec_MSG_BuyItemLeilaoStore(int clientId, char* pMsg)
{
	pStartNegotiation* p = (pStartNegotiation*)(pMsg);

	if (p->Size > sizeof(pStartNegotiation))
	{
		printf(" Maior Pacote: 0x%X - Tamanho: %d - Index: %d\n", p->Type, p->Size, p->ID);
		return;
	}

	STRUCT_MOB* player = (STRUCT_MOB*)(&pMob[clientId].MOB);


	int itemType = p->itemType;
	int slotBank = p->SlotBank;
	int itemSlot = p->itemSlot;
	int itemPrice = p->itemPrice;
	int itemTax = p->itemTax;
	int mobId = p->targetPlayerId;
	STRUCT_ITEM newItem = p->item;

	INT32 DonoLoja = mobId;


	if (!player->CurrentScore.Hp || pUser[clientId].Mode != USER_PLAY)
		return;


	if (DonoLoja <= 0 || DonoLoja >= MAX_USER)
	{
		SendClientMessage(clientId, "indice do lider fora do indice");
		return;
	}

	/*if (DonoLoja == pMob[DonoLoja].AutoTrade.List->OwnerId)
	{
		SendClientMessage(clientId, "Ação não permitida");
		return false;
	}*/

	if (!pMob[DonoLoja].MOB.CurrentScore.Hp || pUser[DonoLoja].Mode != USER_PLAY)
		return;


	if (pUser[clientId].Trade.OpponentID != 0)
	{
		RemoveTrade(clientId);

		AddCrackError(clientId, 1, 810);
		return;
	}


	/*

	// bloqueio distancia

	if (pMob[clientId].Target.X < pMob[mobId].Target.X - VIEWGRIDX || pMob[clientId].Target.X > pMob[mobId].Target.X + VIEWGRIDX || pMob[clientId].Target.Y < pMob[mobId].Target.Y - VIEWGRIDY || pMob[clientId].Target.Y > pMob[mobId].Target.Y + VIEWGRIDY)
   {
	   SendClientMessage(clientId, g_pMessageStringTable[_NN_SoFar_BuyShop]);
	   return;
   }

   */


	if (itemSlot < 0 || itemSlot >= MAX_SLOT_LEILAO_AUTOTRADE)
	{
		return;
	}



	INT32 SellTipy = 0;

	SellTipy = pMob[DonoLoja].AutoTrade.List[itemSlot].Categoria;
	int MyCash = 0;
	int MyGold = player->Coin;
	int MyBis = 0;

	auto& pc = cSQL::instance();


	sprintf(hQuery, "SELECT * FROM `accounts` WHERE `username` = '%s'", pUser[clientId].AccountName);
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		MyCash = atoi(row[6]);
	}
	INT8 MyBisSlot = GetFirstSlot(clientId, 4011);

	if (MyBisSlot != -1)
	{
		MyBis = GetItemAmount(&pMob[clientId].MOB.Carry[MyBisSlot]);
	}

	if (slotBank < 0 || slotBank >= 128)
	{

		return;
	}

	if (itemTax != pMob[DonoLoja].AutoTrade.List[itemSlot].TaxRate)
	{
		return;
	}

	printf(" fluxo Pacote:   %d %d \n", itemPrice, pMob[DonoLoja].AutoTrade.List[itemSlot].Price);

	if (itemPrice != pMob[DonoLoja].AutoTrade.List[itemSlot].Price)
	{
		return;
	}

	if (pMob[DonoLoja].AutoTrade.List[itemSlot].Item.sIndex == 0)
	{
		return;
	}

	INT32 itemCheck = memcmp(&p->item, &pMob[DonoLoja].AutoTrade.List[itemSlot].Item, 8);


	if (itemCheck != 0)
	{
		return;
	}

	itemCheck = memcmp(&pUser[DonoLoja].Cargo[slotBank], &pMob[DonoLoja].AutoTrade.List[itemSlot].Item, 8);

	if (itemCheck != 0)
	{
		return;
	}

	if (SellTipy == 1) // o preço do item é em donate
	{

		if (MyCash < itemPrice)
		{
			SendClientMessage(clientId, "Não possui a quantia de cash!");
			return;
		}
	}
	else if (SellTipy == 2) // o preço do item é em gold
	{
		if (MyGold < itemPrice)
		{
			SendClientMessage(clientId, "Não possui a quantia de gold!");
			return;
		}
	}
	else if (SellTipy == 3) // o preço do item é em biss
	{
		if (MyBis < itemPrice)
		{
			SendClientMessage(clientId, "Não possui a quantia de bis!");
			return;
		}
	}
	else
	{
		if (player->Coin < itemPrice)
		{
			SendClientMessage(clientId, "Não possui a quantia de gold!");
			return;
		}
	}




	if (itemPrice <= 1000 && SellTipy == 2)// o preço do item é em gold
	{
		SendClientMessage(clientId, "Este item esta em modo demonstração");

		return;
	}

	STRUCT_ITEM* item = (STRUCT_ITEM*)&p->item;

	INT32 nullSlot = GetFirstSlot(clientId, 0);
	if (nullSlot < 0 || nullSlot >= MAX_CARRY)
	{
		SendClientMessage(clientId, g_pMessageStringTable[_NN_You_Have_No_Space_To_Trade]);
		return;
	}

	INT32 impost = 0;
	INT32 totalPrice = itemPrice; // local1190

	INT32 mobTargetX = pMob[mobId].TargetX; // local1178
	INT32 mobTargetY = pMob[mobId].TargetY; // local1179
	INT32 villageId = BASE_GetVillage(mobTargetX, mobTargetY); // local1180

	if (villageId < 0 || villageId >= 5)
	{ // Permitido somente dentro da cidade
		SendClientMessage(clientId, g_pMessageStringTable[_NN_OnlyVillage]);
		return;
	}

	INT32 tax = g_pGuildZone[villageId].CityTax;

	if (SellTipy == 2 && itemPrice >= 100000)
	{
		impost = itemPrice / 100 * tax;

		totalPrice = itemPrice - impost;
	}

	UINT64 total = pUser[DonoLoja].Coin + totalPrice;
	if (total > 2000000000)
	{
		SendClientMessage(clientId, g_pMessageStringTable[_NN_Cant_get_more_than_2G]);

		return;
	}


	memcpy(&player->Carry[nullSlot], item, 8);

	SendItem(clientId, ITEM_PLACE_CARRY, nullSlot, &player->Carry[nullSlot]);

	pMob[DonoLoja].AutoTrade.List[itemSlot].SlotBank = -1; // -1;

	memset(&pMob[DonoLoja].AutoTrade.List[itemSlot].Item, 0, 8);

	pMob[DonoLoja].AutoTrade.List[itemSlot].Price = 0;
	memset(&pUser[DonoLoja].Cargo[slotBank], 0, 8);


	SendItem(DonoLoja, ITEM_PLACE_CARGO, slotBank, &pUser[DonoLoja].Cargo[slotBank]);


	if (SellTipy == 1) // o preço do item é em donate
	{
		auto& pc = cSQL::instance();
		int Donate = 0;
		sprintf(hQuery, "SELECT * FROM `accounts` WHERE `username` = '%s'", pUser[clientId].AccountName);
		MYSQL_ROW row;
		MYSQL* wSQL = pc.wStart();
		MYSQL_RES* result = pc.wRes(wSQL, hQuery);

		if (result == NULL) {
			return;
		}


		while ((row = mysql_fetch_row(result)) != NULL)
		{
			Donate = atoi(row[6]) - itemPrice;
		}

		sprintf(xQuery, "UPDATE accounts SET donate = '%d' WHERE username = '%s' ", (Donate), pUser[clientId].AccountName);
		pc.wQuery(xQuery);


		UpdateDonate(clientId);
		SaveUser(clientId, 0);


		auto& pc2 = cSQL::instance();
		int SellDonate = 0;
		sprintf(hQuery, "SELECT * FROM `accounts` WHERE `username` = '%s'", pUser[DonoLoja].AccountName);
		MYSQL_ROW row2;
		MYSQL* wSQL2 = pc2.wStart();
		MYSQL_RES* result2 = pc2.wRes(wSQL2, hQuery);

		if (result2 == NULL) {
			return;
		}


		while ((row2 = mysql_fetch_row(result2)) != NULL)
		{
			SellDonate = atoi(row2[6]) + itemPrice;
		}

		sprintf(xQuery, "UPDATE accounts SET donate = '%d' WHERE username = '%s' ", (SellDonate), pUser[DonoLoja].AccountName);
		pc2.wQuery(xQuery);

		UpdateDonate(DonoLoja);
		SaveUser(DonoLoja, 0); 

	}
	else if (SellTipy == 2) // o preço do item é em gold
	{
		player->Coin = player->Coin - itemPrice;
		SendCarry(clientId);

		if (pUser[DonoLoja].Coin < 2000000000)
		{
			pUser[DonoLoja].Coin += totalPrice;
			SendCargoCoin(DonoLoja);
		}

	}
	else if (SellTipy == 3) // o preço do item é em biss
	{
		if (MyBisSlot == -1)//não tem bis
		{
			return;
		}

		AmountMinusPlus(&pMob[clientId].MOB.Carry[MyBisSlot], itemPrice);
		SendItem(clientId, ITEM_PLACE_CARRY, MyBisSlot, &pMob[clientId].MOB.Carry[MyBisSlot]);

		for (int i = 0; i < MAX_CARGO; i++)
		{
			if (pUser[DonoLoja].Cargo[i].sIndex != 0)
				continue;

			pUser[DonoLoja].Cargo[i].sIndex = 4011;
			pUser[DonoLoja].Cargo[i].stEffect[0].cEffect = 61;
			pUser[DonoLoja].Cargo[i].stEffect[0].cValue = itemPrice;
			SendItem(DonoLoja, ITEM_PLACE_CARGO, i, &pUser[DonoLoja].Cargo[i]);
			break;
		}

	}
	else
	{
		SendClientMessage(clientId, "Invalido!");
		return;
	}


	struct p39B
	{
		_MSG;
		INT32 MobID;
		INT32 SlotID;
	};

	char strtemp[128]{};
	snprintf(strtemp, 128, "!O personagem [%s] comprou o item %s por %d", player->MobName, g_pItemList[p->item.sIndex].Name, p->itemPrice);

	SendClientMessage(p->targetPlayerId, strtemp);

	printf("\nComprou o item %s [%d] [%d %d %d %d %d %d] de %s (%s) por %d. Gold sobrando: %d - Impostos pago: %d", g_pItemList[item->sIndex].Name, item->sIndex, item->stEffect[0].cEffect,
		item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue, pMob[DonoLoja].MOB.MobName, pUser[DonoLoja].AccountName, p->itemPrice,
		player->Coin, impost);

	printf("\nVendeu o item %s [%d] [%d %d %d %d %d %d] para %s (%s) por %d. Gold banco: %d - Impostos: %d", g_pItemList[item->sIndex].Name, item->sIndex, item->stEffect[0].cEffect,
		item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue, player->MobName, pUser[clientId].AccountName, p->itemPrice, pUser[DonoLoja].Coin,
		impost);

	printf("\nComprou o item %s da loja de %s por %d", g_pItemList[item->sIndex].Name, pMob[p->targetPlayerId].MOB.MobName, p->itemPrice);
	printf("\nVendeu o item %s de sua loja para %s por %d. %d foram pagos de impostos", g_pItemList[item->sIndex].Name, pMob[clientId].MOB.MobName, p->itemPrice, impost);


	snprintf(strtemp, 128, "Comprou o item %s de %s por %d", g_pItemList[item->sIndex].Name, pMob[DonoLoja].MOB.MobName, p->itemPrice);
	SendClientMessage(clientId, strtemp);

	snprintf(strtemp, 128, "Vendeu o item %s de %s por %d. %d de impostos", g_pItemList[item->sIndex].Name, pMob[clientId].MOB.MobName, p->itemPrice, impost);
	SendClientMessage(DonoLoja, strtemp);


	if (impost > 50000000)
	{
		printf("\nPagos um total de %d de impostos... Valor maior que 50 milhões.", impost);
		printf("\n[%s] Vendeu o item %s [%d] [%d %d %d %d %d %d] para %s (%s) por %d. Gold banco: %d - Impostos: %d", pUser[p->targetPlayerId].AccountName, g_pItemList[item->sIndex].Name, item->sIndex, item->stEffect[0].cEffect,
			item->stEffect[0].cValue, item->stEffect[1].cEffect, item->stEffect[1].cValue, item->stEffect[2].cEffect, item->stEffect[2].cValue, player->MobName, pUser[clientId].AccountName, p->itemPrice, pUser[DonoLoja].Coin,
			impost);
	}


	UpdateLeilaoSaldo(mobId);
	UpdateLeilaoSaldo(clientId);

	snprintf(strtemp, 128, "Comprou %s de %s($:%d)", g_pItemList[p->item.sIndex].Name, pMob[mobId].MOB.MobName, p->itemPrice);
	SendClientMessage(clientId, strtemp);

	snprintf(strtemp, 128, "Vendeu %s de %s($:%d) %d de impostos", g_pItemList[p->item.sIndex].Name, pMob[clientId].MOB.MobName, p->itemPrice, itemTax);
	SendClientMessage(mobId, strtemp);
	SaveUser(clientId, 0);
	SaveUser(mobId, 0);
	SendLeilaoStoreAlvoInfos(clientId, mobId);
	SendLeilaoStoreMyInfos(mobId);


	return;

}















///*Negociação de itens no Sistema de leilão*/
//void Exec_MSG_BuyItemLeilaoStore(int clientId, char* pMsg)
//{
//	pStartNegotiation* p = (pStartNegotiation*)(pMsg);
//
//	// Valida o tamanho do pacote
//	if (p->Size != sizeof(pStartNegotiation))
//	{
//		printf("Pacote inválido: 0x%X - Tamanho: %d - Index: %d\n", p->Type, p->Size, p->ID);
//		return;
//	}
//
//	STRUCT_MOB* player = (STRUCT_MOB*)(&pMob[clientId].MOB);
//
//	int itemType = p->itemType;
//	int slotBank = p->SlotBank;
//	int itemSlot = p->itemSlot;
//	int itemPrice = p->itemPrice;
//	int itemTax = p->itemTax;
//	int mobId = p->targetPlayerId;
//	STRUCT_ITEM newItem = p->item;
//
//
//	std::cout << "  *******************  " << "  \n  ";
//	std::cout << "\n  Compra recebida :: itemSlot " << itemSlot;
//	std::cout << "  mobId " << mobId;
//	std::cout << "  Price " << static_cast<int>(itemPrice);
//	std::cout << "  Categoria " << static_cast<int>(itemType);
//	std::cout << "  TaxRate " << static_cast<int>(itemTax);
//	std::cout << "  SlotBank  " << static_cast<int>(slotBank) << "  \n  ";
//	std::cout << "  *******************  "  << "  \n  ";
//
//	// Verifica se o jogador está ativo e possui permissão para comprar
//	if (!player->CurrentScore.Hp || pUser[clientId].Mode != USER_PLAY)
//	{
//		SendClientMessage(clientId, "Você não está em um estado válido para realizar a compra.");
//		return;
//	}
//
//	// Verifica se o ID do dono da loja é válido
//	if (mobId <= 0 || mobId >= MAX_USER)
//	{
//		std::cout << "ID do dono da loja inválido.\n";
//		return;
//	}
//
//	// Verifica se o dono da loja está ativo
//	if (!pMob[mobId].MOB.CurrentScore.Hp || pUser[mobId].Mode != USER_PLAY)
//	{
//		std::cout << "O dono da loja não está ativo.\n" ;
//		return;
//	}
//
//
//	// Verifica se o dono da loja está comprando o proprio item
//	if (mobId == clientId)
//	{
//		std::cout << "O dono da loja comprando os proprios itens.\n";
//		return;
//	}
//
//
//	// Verifica a validade do slot e do item
//	if (itemSlot < 0 || itemSlot >= MAX_SLOT_LEILAO_AUTOTRADE)
//	{
//		std::cout << "Slot do item inválido.\n" ;
//		return;
//	}
//
//	INT32 verify = memcmp(&pUser[mobId].Cargo[slotBank], &newItem, 8);
//
//	if (verify != 0)
//	{
//		std::cout << "verify do item inválido.\n"  << slotBank;
//		return;
//	}
//
//	// Verifica se o slot contém um item e se as informações estão corretas
//	auto& itemList = pMob[mobId].AutoTrade.List[itemSlot];
//
//
//	std::cout << "\n  *******************  ";
//
//	std::cout << "  Item.sIndex " << static_cast<int>(pMob[mobId].AutoTrade.List[itemSlot].Item.sIndex);
//	std::cout << "\n  OwnerId recebida :: itemSlot " << itemSlot;
//	std::cout << "  mobId " << pMob[mobId].AutoTrade.List[itemSlot].OwnerId;
//	std::cout << "  Price " << static_cast<int>(pMob[mobId].AutoTrade.List[itemSlot].Price);
//	std::cout << "  Categoria " << static_cast<int>(pMob[mobId].AutoTrade.List[itemSlot].Categoria);
//	std::cout << "  TaxRate " << static_cast<int>(pMob[mobId].AutoTrade.List[itemSlot].TaxRate);
//	std::cout << "  SlotBank  " << static_cast<int>(pMob[mobId].AutoTrade.List[itemSlot].SlotBank) << "  \n  ";
//
//	std::cout << "  *******************  " << "  \n  ";
//
//	if (itemList.Item.sIndex == 0 || itemList.Categoria != itemType)
//	{
//		std::cout << "Item não encontrado ou categoria inválida.\n" ; 
//		return;
//	}
//
//	// Verifica se o preço e taxa correspondem
//	if (itemList.Price != itemPrice || itemList.TaxRate != itemTax)
//	{
//		std::cout << "Preço ou taxa do item não correspondem.\n" ;
//		return;
//	}
//
//	
//	/*Sistema de imposto se quiser usar*/
//	/*
//	
//	int imposto = 0;
//   	int PriceEnd = itemPrice;
//
//	if (itemType == 2)
//	{ 
//		if (itemPrice >= 100000)
//		{
//			imposto = (itemPrice / 100) * itemTax;
//			PriceEnd = itemPrice - imposto;
//		}
//	}
//	if(PriceEnd > 0)
//	{
//		itemPrice = PriceEnd;
//	}
//
//	*/
//
//	// Verifica se o item no banco corresponde
//	if (memcmp(&newItem, &itemList.Item, sizeof(STRUCT_ITEM)) != 0)
//	{
//		std::cout << "O item selecionado não corresponde ao item listado.\n";
//		return;
//	}
//
//	// Verifica se o jogador tem fundos suficientes
//	int playerFunds = 0;
//	if (itemType == 1) // Donate
//	{  
//		ProcessarBuyLeilaoStore(clientId, mobId, itemPrice);  
//	}
//	else if (itemType == 2) // Gold
//	{
//		if (player->Coin < itemPrice)
//		{
//			SendClientMessage(clientId, "Não há fundos suficientes de gold.");
//			return;
//		}
//	}
//	else if (itemType == 3) // Biss
//	{
//		// Verificar fundos de biss
//		int bissSlot = GetFirstSlot(clientId, 4011);
//		if (bissSlot == -1 || GetItemAmount(&pMob[clientId].MOB.Carry[bissSlot]) < itemPrice)
//		{
//			SendClientMessage(clientId, "Não há fundos suficientes de biss.");
//			return;
//		}
//	}
//
//	// Processar a compra 
//	// Exemplo de transferência e atualização de fundos:
//	int nullSlot = GetFirstSlot(clientId, 0);
//	if (nullSlot < 0 || nullSlot >= MAX_CARRY)
//	{
//		SendClientMessage(clientId, "Você não tem espaço suficiente para a compra.");
//		return;
//	}
//	 
//	STRUCT_ITEM* item = (STRUCT_ITEM*)&p->item;
//	 
//	memcpy(&player->Carry[nullSlot], item, 8);
//	SendItem(clientId, ITEM_PLACE_CARRY, nullSlot, &player->Carry[nullSlot]);
//
//
//	memset(&pMob[mobId].AutoTrade.List[itemSlot].Item, 0, 8);
//	pMob[mobId].AutoTrade.List[itemSlot].Price = 0;
//	pMob[mobId].AutoTrade.List[itemSlot].TaxRate = 0;
//	pMob[mobId].AutoTrade.List[itemSlot].Categoria = 0;
//	pMob[mobId].AutoTrade.List[itemSlot].OwnerId = 0;
//	pMob[mobId].AutoTrade.List[itemSlot].SlotBank = -1; // -1;
//
//	memset(&pUser[mobId].Cargo[slotBank], 0, 8);
//
//
//	SendItem(mobId, ITEM_PLACE_CARGO, slotBank, &pUser[mobId].Cargo[slotBank]);
//	 
//
//	// Atualizar saldo do jogador e do dono da loja
//	if (itemType == 1) // Donate
//	{
//		auto& pc = cSQL::instance(); 
//		int debitCash = playerFunds - itemPrice;
//		snprintf(xQuery, sizeof(xQuery), "UPDATE accounts SET donate = '%d' WHERE username = '%s'", debitCash, pUser[clientId].AccountName);
//		pc.wQuery(xQuery);
//		UpdateDonate(clientId);
//
//		snprintf(xQuery, sizeof(xQuery), "UPDATE accounts SET donate = '%d' WHERE username = '%s'", playerFunds + itemPrice, pUser[mobId].AccountName);
//		pc.wQuery(xQuery);
//		UpdateDonate(mobId);
//		 
//	}
//	else if (itemType == 2) // Gold
//	{
//		player->Coin -= itemPrice;
//		pUser[mobId].Coin += itemPrice; 
//	}
//	else if (itemType == 3) // Biss
//	{ 
//		AmountMinusPlus(&pMob[clientId].MOB.Carry[GetFirstSlot(clientId, 4011)], itemPrice);
//		SendItem(clientId, ITEM_PLACE_CARRY, GetFirstSlot(clientId, 4011), &pMob[clientId].MOB.Carry[GetFirstSlot(clientId, 4011)]);
//		for (int i = 0; i < MAX_CARGO; i++)
//		{
//			if (pUser[mobId].Cargo[i].sIndex == 0)
//			{
//				pUser[mobId].Cargo[i].sIndex = 4011;
//				pUser[mobId].Cargo[i].stEffect[0].cEffect = 61;
//				pUser[mobId].Cargo[i].stEffect[0].cValue = itemPrice;
//				SendItem(mobId, ITEM_PLACE_CARGO, i, &pUser[mobId].Cargo[i]);
//				break;
//			}
//		}
//	} 
//
//	UpdateLeilaoSaldo(mobId);
//	UpdateLeilaoSaldo(clientId);
//
//	char strtemp[128]{}; 
//	 
//	snprintf(strtemp, 128, "Comprou %s de %s($:%d)", g_pItemList[p->item.sIndex].Name, pMob[mobId].MOB.MobName, p->itemPrice);
//	SendClientMessage(clientId, strtemp);
//
//	snprintf(strtemp, 128, "Vendeu %s de %s($:%d) %d de impostos", g_pItemList[p->item.sIndex].Name, pMob[clientId].MOB.MobName, p->itemPrice, itemTax);
//	SendClientMessage(mobId, strtemp);
//	SaveUser(clientId, 0);
//	SaveUser(mobId, 0); 
//	SendLeilaoStoreMyInfos(clientId);
//	SendLeilaoStoreMyInfos(mobId);
//	return;
//}
//void ProcessarBuyLeilaoStore(int clientId, int mobId, int itemPrice)
//{
//	auto& pc = cSQL::instance();
//
//	// Variáveis para consulta e resultado
//	int donate = 0;
//	MYSQL_ROW row;
//	MYSQL* wSQL = pc.wStart();
//	MYSQL_RES* result = nullptr;
//	MYSQL_STMT* stmt = nullptr;
//	MYSQL_BIND bind[2];
//
//	// Consulta ao saldo atual usando prepared statements para evitar SQL injection
//	const char* query = "SELECT donate FROM accounts WHERE username = ?";
//
//	stmt = mysql_stmt_init(wSQL);
//	if (!stmt) {
//		fprintf(stderr, "mysql_stmt_init() failed\n");
//		return;
//	}
//
//	if (mysql_stmt_prepare(stmt, query, strlen(query))) {
//		fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
//		mysql_stmt_close(stmt);
//		return;
//	}
//
//	memset(bind, 0, sizeof(bind));
//
//	bind[0].buffer_type = MYSQL_TYPE_STRING;
//	bind[0].buffer = (char*)pUser[clientId].AccountName;
//	bind[0].buffer_length = strlen(pUser[clientId].AccountName);
//
//	if (mysql_stmt_bind_param(stmt, bind)) {
//		fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
//		mysql_stmt_close(stmt);
//		return;
//	}
//
//	if (mysql_stmt_execute(stmt)) {
//		fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
//		mysql_stmt_close(stmt);
//		return;
//	}
//
//	result = mysql_stmt_result_metadata(stmt);
//	if (!result) {
//		fprintf(stderr, "mysql_stmt_result_metadata() failed: %s\n", mysql_stmt_error(stmt));
//		mysql_stmt_close(stmt);
//		return;
//	}
//
//	MYSQL_BIND resultBind[1];
//	memset(resultBind, 0, sizeof(resultBind));
//	resultBind[0].buffer_type = MYSQL_TYPE_LONG;
//	resultBind[0].buffer = &donate;
//
//	if (mysql_stmt_bind_result(stmt, resultBind)) {
//		fprintf(stderr, "mysql_stmt_bind_result() failed: %s\n", mysql_stmt_error(stmt));
//		mysql_stmt_close(stmt);
//		return;
//	}
//
//	if (mysql_stmt_fetch(stmt) != 0 && mysql_stmt_fetch(stmt) != MYSQL_NO_DATA) {
//		fprintf(stderr, "mysql_stmt_fetch() failed: %s\n", mysql_stmt_error(stmt));
//		mysql_stmt_close(stmt);
//		return;
//	}
//
//	mysql_stmt_close(stmt);
//
//	int saldoAtual = donate + itemPrice;
//
//	// Atualizando o saldo no banco de dados
//	const char* updateQuery = "UPDATE accounts SET donate = ? WHERE username = ?";
//
//	stmt = mysql_stmt_init(wSQL);
//	if (!stmt) {
//		fprintf(stderr, "mysql_stmt_init() failed\n");
//		return;
//	}
//
//	if (mysql_stmt_prepare(stmt, updateQuery, strlen(updateQuery))) {
//		fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
//		mysql_stmt_close(stmt);
//		return;
//	}
//
//	memset(bind, 0, sizeof(bind));
//
//	bind[0].buffer_type = MYSQL_TYPE_LONG;
//	bind[0].buffer = &saldoAtual;
//
//	bind[1].buffer_type = MYSQL_TYPE_STRING;
//	bind[1].buffer = (char*)pUser[clientId].AccountName;
//	bind[1].buffer_length = strlen(pUser[clientId].AccountName);
//
//	if (mysql_stmt_bind_param(stmt, bind)) {
//		fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
//		mysql_stmt_close(stmt);
//		return;
//	}
//
//	if (mysql_stmt_execute(stmt)) {
//		fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
//		mysql_stmt_close(stmt);
//		return;
//	}
//
//	mysql_stmt_close(stmt);
//
//	// Atualizando o cliente no jogo
//	UpdateDonate(clientId);
//	SaveUser(clientId, 0);
//
//	// Debugging
//	printf("\nsaldoAtual %d Donate %d itemPrice %d", saldoAtual, donate, itemPrice);
//}


