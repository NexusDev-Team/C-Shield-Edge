# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Operações do Orion Context Broker (porta 1026): leitura, subscriptions e
remoção de entidades. O dashboard só LÊ: não há escrita de dados nem comandos.

Mixin do FiwareClient; depende de `_request` e `orion_url` de FiwareBase.
"""
from app.services.fiware_base import FiwareBase
from app.services.fiware_constants import ATRIBUTOS_LONGOS, ENTITY_TYPE, ORION_LIST_LIMIT


class OrionOperations(FiwareBase):
    """Consulta e escrita de entidades no Orion."""

    # ----- leitura -----------------------------------------------------------

    async def get_entity(self, entity_id: str) -> dict:
        """Estado atual de uma entidade, no formato simplificado keyValues."""
        resposta = await self._request(
            "orion", "GET", self.orion_url(f"/v2/entities/{entity_id}"),
            params={"options": "keyValues"},
        )
        return resposta.json()

    async def list_entities(self, entity_type: str = ENTITY_TYPE) -> list[dict]:
        """Todas as entidades de um tipo numa única chamada (usado pelo poller).

        O TimeInstant vem junto quando o service group tem timestamp: true.
        """
        resposta = await self._request(
            "orion", "GET", self.orion_url("/v2/entities"),
            params={"type": entity_type, "options": "keyValues", "limit": ORION_LIST_LIMIT},
        )
        return resposta.json()

    # ----- subscriptions do histórico ----------------------------------------

    async def subscribe(self, entity_id: str, condicao: str, notificados: tuple[str, ...],
                        descricao: str) -> str:
        """Cria uma subscription Orion -> STH-Comet e devolve o id dela.

        `condicao` é o atributo que dispara; `notificados`, o que o STH grava.
        Para telemetria os dois são o mesmo atributo; para eventos de cartão a
        condição é o contador e a notificação leva o evento inteiro. O destino é o
        endereço interno do STH na rede Docker (STH_INTERNAL_URL).
        """
        for attr in (condicao, *notificados):
            if attr not in ATRIBUTOS_LONGOS:
                raise ValueError(f"atributo inválido: {attr!r}; use um de {ATRIBUTOS_LONGOS}")
        corpo = {
            "description": descricao,
            "subject": {
                "entities": [{"id": entity_id, "type": ENTITY_TYPE}],
                "condition": {"attrs": [condicao]},
            },
            "notification": {
                "http": {"url": f"{self._settings.sth_internal_url}/notify"},
                "attrs": list(notificados),
                "attrsFormat": "legacy",
            },
        }
        resposta = await self._request("orion", "POST", self.orion_url("/v2/subscriptions"),
                                       json=corpo)
        # O Orion devolve o id no header Location: /v2/subscriptions/<id>
        return resposta.headers.get("Location", "").rstrip("/").split("/")[-1]

    async def delete_entity(self, entity_id: str) -> None:
        """Remove a entidade do Orion; se já não existe (404), segue."""
        await self._request("orion", "DELETE", self.orion_url(f"/v2/entities/{entity_id}"),
                            aceitar=(404,))

    async def delete_subscriptions(self, entity_id: str) -> int:
        """Apaga as subscriptions que observam esta entidade e devolve quantas foram."""
        resposta = await self._request("orion", "GET", self.orion_url("/v2/subscriptions"),
                                       params={"limit": ORION_LIST_LIMIT})
        apagadas = 0
        for assinatura in resposta.json():
            entidades = assinatura.get("subject", {}).get("entities", [])
            if any(e.get("id") == entity_id for e in entidades):
                await self._request(
                    "orion", "DELETE",
                    self.orion_url(f"/v2/subscriptions/{assinatura['id']}"),
                    aceitar=(404,),
                )
                apagadas += 1
        return apagadas
