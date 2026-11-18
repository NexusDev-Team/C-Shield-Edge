# C-Shield — Dashboard (terceira camada: lê o FIWARE)
# GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
# Equipe Nexus Vision. Integrantes:
#   Isac Nilton Fernandes de Oliveira — RM573282
#   João Benedito de Oliveira Simplício — RM570206
#   Leonardo Formigari — RM573291
#   Luis Fernando — RM570959
#   Nicolas Cinel — RM572940

"""
Operações do IoT Agent MQTT (porta 4041): service group e devices do C-Shield.

Mixin do FiwareClient; depende de `_request`, `iota_url` e `orion_url`
fornecidos por FiwareBase.
"""
from app.models.schemas import Device
from app.services.fiware_base import FiwareBase
from app.services.fiware_constants import ENTITY_TYPE, EVENT_ATTRS, SENSOR_ATTRS


class IotaOperations(FiwareBase):
    """Provisionamento de service group e devices no IoT Agent."""

    async def provision_service_group(self) -> None:
        """Cria o service group da apikey do projeto; se já existe (409), segue.

        timestamp: true faz o IoT Agent preencher TimeInstant a cada leitura,
        e a detecção de offline depende disso.
        """
        corpo = {"services": [{
            "apikey": self._settings.fiware_apikey,
            "cbroker": self.orion_url(""),
            "entity_type": "Thing",
            "resource": "",
            "timestamp": True,
        }]}
        await self._request("iota", "POST", self.iota_url("/iot/services"),
                            json=corpo, aceitar=(409,))

    async def provision_device(self, device: Device) -> None:
        """Provisiona o C-Shield no IoT Agent, só com atributos.

        Sem "commands": o C-Shield envia dados e não recebe ordens, então não há
        registration de comandos no Orion. Device já existente levanta FiwareConflict.
        """
        corpo = {"devices": [{
            "device_id": device.device_id,
            "entity_name": device.entity_id,
            "entity_type": ENTITY_TYPE,
            "protocol": "PDI-IoTA-UltraLight",
            "transport": "MQTT",
            "attributes": [
                {"object_id": curto, "name": longo, "type": tipo}
                for curto, longo, tipo in SENSOR_ATTRS + EVENT_ATTRS
            ],
        }]}
        await self._request("iota", "POST", self.iota_url("/iot/devices"), json=corpo)

    async def delete_device(self, device_id: str) -> None:
        """Remove o device do IoT Agent; se já não existe (404), segue."""
        await self._request("iota", "DELETE", self.iota_url(f"/iot/devices/{device_id}"),
                            aceitar=(404,))
