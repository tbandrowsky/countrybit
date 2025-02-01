using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace coronanet
{

    public class CoronaService : ICoronaService
    {
        private readonly HttpClient _httpClient;
        private readonly string _remoteServiceBaseUrl;

        public CatalogService(HttpClient httpClient)
        {
            _httpClient = httpClient;
        }

        public async Task<Catalog> GetCatalogItems(int page, int take,
                                                   int? brand, int? type)
        {
            var uri = API.Catalog.GetAllCatalogItems(_remoteServiceBaseUrl,
                                                     page, take, brand, type);

            var responseString = await _httpClient.GetStringAsync(uri);

            var catalog = JsonConvert.DeserializeObject<Catalog>(responseString);
            return catalog;
        }
    }
}
